#include "comio2.h"


//
//
// MÃ©thodes pour le protocole V1
//
//
#ifdef _COMIO1_COMPATIBLITY_MODE_

void Comio2::init(unsigned char io_defs[][3])
{
  if(!io_defs)
    return;

  // initialisation des entrÃ©es/sorties
  for(int i=0;io_defs[i][0]!=255;i++)
  {
    switch(io_defs[i][2])
    {
    case TYPE_DIGITAL:
      comio_digitals[io_defs[i][0]]=io_defs[i][1];
      if(io_defs[i][1]==OP_WRITE)
        pinMode((int)io_defs[i][0],OUTPUT);
      else if(io_defs[i][1]==OP_READ)
        pinMode((int)io_defs[i][0],INPUT);
      break;

    case TYPE_ANALOG:
      if(io_defs[i][1]==OP_READ)
      {
        comio_analogs[io_defs[i][0]]=io_defs[i][1];
        pinMode((int)io_defs[i][0],INPUT);
      }
      break;

    case TYPE_DIGITAL_PWM:
      if(io_defs[i][1]==OP_WRITE)
      {
        comio_digitals[io_defs[i][0]]=io_defs[i][1];
        analogWrite((int)io_defs[i][0],0);
      }
      break;
    }
  }
}


void Comio2::setFunction(unsigned char num_function, callback_f function)
/**
 * \brief     Association d'une fonction Ã  un "port" (numÃ©ro de fonction).
 * \details  
 * \param     num_function  numÃ©ro de fonction
 * \param     function      fonction associÃ©e au numÃ©ro / mettre NULL pour desalouer une fonction
 */
{
  if(num_function<COMIO_MAX_FX)
    comio_functions[num_function]=function;
}


void Comio2::sendTrap(unsigned char num_trap)
/**
 * \brief     Ã©mission d'une trame TRAP de type "Comio1" dans le buffer de sortie.
 * \details   Ã©mission d'une trame TRAP de type "Comio1" dans le buffer de sortie.
 * \param     num_trap  numÃ©ro du trap
 * \param     value     donnÃ©es d'accompagnement du trap
 * \param     l_value   nombre de donnÃ©es dans le champ valeur
 */
{
  writeF('*');
  writeF(num_trap);
  writeF('$');
  flushF();
}


void Comio2::sendLongTrap(unsigned char num_trap, char *value, char l_value)
/**
 * \brief     Ã©mission d'une trame TRAP LONG de type "Comio1" dans le buffer de sortie.
 * \details   Construit une trame de TRAP LONG et l'envoie dans le buffer de sortie
 * \param     num_trap  numÃ©ro du trap
 * \param     value     donnÃ©es d'accompagnement du trap
 * \param     l_value   nombre de donnÃ©es dans le champ valeur
 */
{
  writeF('#');
  writeF(num_trap);
  writeF(l_value);
  for(int i=0;i<l_value;i++)
    writeF(value[i]);
  writeF('$');
  flushF();
}


void Comio2::_comio_send_frame(unsigned char op, unsigned char var, unsigned char type, unsigned int val)
/**
 * \brief     Ã©mission d'une trame "rÃ©ponse" de type "Comio1" dans le buffer de sortie.
 * \details   Construit une trame de rÃ©ponse et l'envoie dans le buffer de sortie
 * \param     op    operation Ã  rÃ©aliser (sur 4 bits : valeur de 0 Ã  16) / valeurs dÃ©finie : OP_READ, OP_WRITE, OP_FUNCTION
 * \param     var   adresse de la variable (adresse mÃ©moire, PIN, ...) sur 8 bits:
 * \param     type  type de l'opÃ©ration (sur 4 bits : valeur de 0 Ã  16) / valuers dÃ©finie TYPE_DIGITAL, TYPE_ANALOG, TYPE_DIGITAL_PWM, TYPE_MEMORY, TYPE_FUNCTION
 * \param     val   valeur numÃ©rique sur 16 bits Ã  retourner (donnÃ©e de l'adresse, valeur de la PIN, ...)
 */
{
  unsigned char op_type;

  op_type=(op & 0x0F) << 4;
  op_type=op_type | (type & 0x0F); 

  unsigned char phigh=(unsigned char)(val/256);
  unsigned char plow=(unsigned char)(val-(unsigned int)(phigh*256));

  writeF('&');
  writeF(op_type);
  writeF(var);
  writeF(phigh);
  writeF(plow);
  writeF('$');
  flushF();
}


void Comio2::_comio_send_error_frame(unsigned char nerr)
/**
 * \brief     Ã©mission d'une trame "erreur" de type "Comio1" dans le buffer de sortie.
 * \details   Construit une trame d'erreur et l'envoie dans le buffer de sortie
 * \param     nerr   numÃ©ro d'erreur Ã  retourner sur 8 bits.
 */
{
  writeF('!');
  writeF(nerr);
  writeF('$');
}


int Comio2::_comio_read_frame(unsigned char *op, unsigned char *var, unsigned char *type, unsigned int *val)
/**
 * \brief     lecture d'une trame "commande" de type "Comio1" dans le buffer d'entrÃ©e.
 * \details   Lit un flot d'entrÃ©e, l'analyse et le dÃ©coupe si la trame est correctement formatÃ©e
 * \param     op    operation Ã  rÃ©aliser (sur 4 bits : valeur de 0 Ã  16)
 * \param     var   adresse de la variable (adresse mÃ©moire, PIN, ...) sur 8 bits:
 * \param     type  type de l'opÃ©ration (sur 4 bits : valeur de 0 Ã  16)
 * \param     val   valeur numÃ©rique sur 16 bits Ã  traiter (donnÃ©e de l'adresse, valeur de la PIN, ...)
 * \return    0 = pas de trame correcte, 1 = ok donnÃ©es disponibles
 */
{
  int c;
  unsigned char stat=0;

  while(stat<5)
  {
    // 5 essais pour obtenir des donnÃ©es
    for(int i=0;i<5;i++)
    {
      c=readF();
      if(c>=0)
        break;
      delay(1); // 1 milliseconde entre 2 lectures
    }

    if(c<0)
      return 0; // pas de donnÃ©es => sortie direct "sans rien dire"

    // automate Ã  Ã©tats pour traiter les diffÃ©rents champs de la trame
    switch(stat)
    {
    case 0: // lecture de l'opÃ©ration et du type
      *op=(((unsigned char)c) & 0xF0) >> 4;
      *type=(((unsigned char)c) & 0x0F);
      stat++; // Ã©tape suivante Ã  la prochaine itÃ©ration
      break;
    case 1: // lecturde de l'adresse de la variable
      *var=(unsigned char)c;
      stat++;
      break;
    case 2: // lecture data poid fort
      *val=c*256;
      stat++;
      break;
    case 3: // lecture data poid faible
      *val=*val+c;
      stat++;
      break;
    case 4: // lecture fin de trame
      if(c!='$')
        return 0; // pas de fin de trame correct ... on oubli tout et on sort
      stat++;
      break;
    }
  }
  return 1;
}


int Comio2::_comio_valid_operation(unsigned char op, unsigned char var, unsigned char type, unsigned int val)
/**
 * \brief     contrÃ´le de la validitÃ© des donnÃ©es d'une demande.
 * \details   VÃ©rifie que les donnÃ©es sont cohÃ©rentes en fonction des plages et des dÃ©clarations effectuÃ©es
 * \param     op    operation Ã  rÃ©aliser (sur 4 bits : valeur de 0 Ã  16)
 * \param     var   adresse de la variable (adresse mÃ©moire, PIN, ...) sur 8 bits:
 * \param     type  type de l'opÃ©ration (sur 4 bits : valeur de 0 Ã  16)
 * \param     val   valeur numÃ©rique sur 16 bits Ã  traiter (donnÃ©e de l'adresse, valeur de la PIN, ...)
 * \return    0 = ok, autre valeur = code d'erreur
 */
{
  switch(type)
  {
  case TYPE_DIGITAL:
    if(comio_digitals[var]!=op)
      return 10;
    break;
  case TYPE_DIGITAL_PWM:
    if(comio_digitals[var]!=op)
      return 11;
    break;
  case TYPE_ANALOG:
    if(comio_analogs[var]!=op)
      return 12;
    break;
  case TYPE_MEMORY:
    if(var>=COMIO_MAX_M)
      return 13;
    break;
  case TYPE_FUNCTION:
    if(var>=COMIO_MAX_FX)
      return 14;
    break;
  default:
    return COMIO_ERR_UNKNOWN; // type inconnu
  }   
  return 0;
}


boolean Comio2::_comio_do_operation(unsigned char op, unsigned char var, unsigned char type, unsigned int val)
/**
 * \brief     rÃ©alise l'opÃ©ration demandÃ© par une commande de type "Comio1".
 * \details   AprÃ¨s une validation, les opÃ©rations peuvent Ãªtre rÃ©alsÃ©es et renvoie une rÃ©ponse.
 * \param     op    operation Ã  rÃ©aliser (sur 4 bits : valeur de 0 Ã  16)
 * \param     var   adresse de la variable (adresse mÃ©moire, PIN, ...) sur 8 bits:
 * \param     type  type de l'opÃ©ration (sur 4 bits : valeur de 0 Ã  16)
 * \param     val   valeur numÃ©rique sur 16 bits Ã  traiter (donnÃ©e de l'adresse, valeur de la PIN, ...)
 * \return    true = ok, false = ko
 */
{
  unsigned int retval=0;

  if(op==OP_WRITE)
  {
    switch(type)
    {
    case TYPE_DIGITAL:
      if(!val)
        digitalWrite(var,LOW);
      else
        digitalWrite(var,HIGH);
      break;
    case TYPE_DIGITAL_PWM:
      analogWrite(var,val);
      break;
    case TYPE_MEMORY:
      comio_memory[var]=(unsigned char)val;
      break;
    default:
      return false;
    }
    retval=val;
  }
  else if (op==OP_READ)
  {
    switch(type)
    {
    case TYPE_ANALOG:
      retval=analogRead(var);
      break;
    case TYPE_DIGITAL:
      retval=digitalRead(var);
      break;
    case TYPE_MEMORY:
      retval=comio_memory[var];
      break;
    default:
      return false;
    }
  }
  else if (op==OP_FUNCTION)
  {
    if(var<COMIO_MAX_FX)
    {
      if(comio_functions[var])
        retval=comio_functions[var](val);
    }
  }

  _comio_send_frame(op,var,type,retval);

  return true;
}

#endif


//
//
// Nouvelles mÃ©thodes pour le protocole V2
//
//

void Comio2::_comio_send_errorV2(unsigned char id, unsigned char error)
/**
 * \brief     envoie une rÃ©ponse signalant une erreur.
 * \details  
 * \param     id    identifiant de la demande qui a provoquÃ© une erreur
 * \param     error numÃ©ro de l'erreur
 */
{
  int cchecksum=0;

  _comio_debut_trameV2(id, COMIO2_CMD_ERROR, 1, &cchecksum);
  cchecksum+=error;
  writeF(error);
  _comio_fin_trameV2(&cchecksum);
}


int Comio2::_comio_read_frameV2(unsigned char *id, unsigned char *cmd, char *data, int *l_data)
/**
 * \brief     lecture d'une trame "commande" de type "Comio2" dans le buffer d'entrÃ©e.
 * \details   Lit un flot d'entrÃ©e, l'analyse et le dÃ©coupe si la trame est correctement formatÃ©e
 * \param     cmd    code commande
 * \param     data   donnÃ©es de la commande
 * \param     l_data nombre de caractÃ¨re de la zone data (max 80 octets)
 * \return    0 => trame correcte,  < 0 => erreur
 */
{
  /*
   Structure d'une trame "question" de type "Comio2"
   
   Start(1)[[] : octet de dÃ©but de trame. Permet d'identifier le type de trame
   Longueur(1) : nombre d'octet de la trame (inclus start et fin de trame)
   Id_trame(1) : numÃ©ro de trame question (0 = pas de rÃ©ponse attendu, sauf erreur)
   Commande(1) : opÃ©ration Ã  rÃ©aliser
   Data(x)     : donnÃ©es de l'opÃ©ration (0<=x<80)
   CheckSum(1) : checksum calculer sur la trame hors start et end (0xFF - (somme_de_tous_les_octets & 0xFF)).
   End(1)[]]   : octet de fin de trame.
   */
  int c=0;
  uint16_t j=0;
  uint8_t stat=0;
  int cchecksum=0;

  while(stat<6)
  {
    // 5 ms max pour obtenir une donnÃ©es
    for(int i=0;i<5;i++)
    {
      c=readF();
      if(c>=0)
        break;
      delay(1); // 5 millisecondes max entre 2 lectures
    }

    if(c<0)
      return 0; // pas de donnÃ©es => sortie direct "sans rien dire"

    // automate Ã  Ã©tats pour traiter les diffÃ©rents champs de la trame
    switch(stat)
    {
    case 0: // lecture de la longueur de trame
      *l_data=c-5;
      cchecksum+=c;
      stat++; // Ã©tape suivante Ã  la prochaine itÃ©ration
      break;

    case 1: // lecturde de l'id de trame
      *id=c;
      cchecksum+=c;
      stat++;
      break;

    case 2: // lecture de la commande
      *cmd=c;
      cchecksum+=c;
      j=0;
      stat++;
      break;

    case 3: // lecture des donnÃ©es
      if(j<COMIO2_MAX_DATA)
      {
        data[j]=c;
        cchecksum+=c;
        j++;
        if(j>=*l_data)
          stat++;
      }
      else
        return 0; // overflow ...
      break;

    case 4: // lecture checksum
      cchecksum+=(unsigned char)c;
      stat++;
      break;

    case 5:
      if(c!=']')
        return 0; // pas de fin de trame correct ... on oubli tout et on sort
      if((cchecksum & 0xFF) != 0xFF)
        return 0; // checksum incorrect ... on oubli tout et on sort
      stat++;
      break;
    }
  }
  return -1;
}


void Comio2::_comio_debut_trameV2(unsigned char id, unsigned char cmd, int l_data, int *cchecksum)
/**
 * \brief     Construit et envoie les octets de dÃ©but d'une trame (checksum + '}') et met Ã  jour le cchecksum Ã  partir des Ã©lÃ©ments en paramÃ¨tre.
 * \param  id      id de la trame
 * \param  cmd     commande (type de trame)
 * \param  l_data  nombre d'octet de la partie donnÃ©es de la trame
 */
{
  writeF('{');

  *cchecksum=6+l_data;
  writeF(6+l_data);

  *cchecksum+=id;
  writeF(id);

  *cchecksum+=cmd;
  writeF(cmd);
}


void Comio2::_comio_fin_trameV2(int *cchecksum)
/**
 * \brief     Construit et envoie les octets de cloture d'une trame (checksum + '}').
 * \param     cchecksum  Ã©tat du checksum jusqu'Ã  prÃ©sent
 */
{
  *cchecksum=0xFF - (*cchecksum & 0xFF);
  writeF(*cchecksum);

  writeF('}');
  flushF();
}


int Comio2::_comio_do_operationV2(unsigned char id, unsigned char cmd, char *data, int l_data)
/**
 * \brief     traite une demande de type "Comio2".
 * \details   Traite une commande (cmd)
 * \param     cmd    code commande
 * \param     data   donnÃ©es de la commande
 * \param     l_data nombre de caractÃ¨re de la zone data (max 80 octets)
 * \return    0 => trame correcte,  < 0 => erreur
 */
{
  /*                                       
   Start(1)[{] : octet de dÃ©but de trame. Permet d'identifier le type de trame
   Longueur(1) : longueur de la rÃ©ponse (start et end compris)
   Id_trame(1) : rÃ©ponse Ã  la question id_trame
   Response(1) : type de la rÃ©ponse
   Data(x)     : donnÃ©es de la rÃ©ponse
   CheckSum(1) : checksum calculÃ© sur la trame hors start et end.
   End(1)[}]   : octet de fin de trame.
   */
  int cchecksum = 0;
  int j=0; // compteur du nombre d'octet Ã©crit

  switch(cmd)
  {
  case COMIO2_CMD_READMEMORY: // memory read - data contient la liste des cases mÃ©moires Ã  lire
    if(!id)
      return 0;

    _comio_debut_trameV2(id, COMIO2_CMD_READMEMORY, l_data, &cchecksum);
    for(int i=0;i<l_data;i++)
    {
      cchecksum+=comio_memory[data[i]];
      writeF(comio_memory[data[i]]);
    }
    _comio_fin_trameV2(&cchecksum);
    return -1;
    break;

  case COMIO2_CMD_WRITEMEMORY: // memory write - data contient la liste des couples Adresse/valeur
    if((l_data % 2) != 0) // parametre impair => erreur
    {
      if(id)
        _comio_send_errorV2(id, COMIO2_ERR_PARAMS);
      return 0;
    }

    for(int i=0; i<l_data; i+=2)
    {
      comio_memory[data[i]]=data[i+1];
      j++;
    }

    if(id)
    {
      _comio_debut_trameV2(id, COMIO2_CMD_WRITEMEMORY, 1, &cchecksum);
      cchecksum+=j;
      writeF(j); // nombre d'octets Ã©crit
      _comio_fin_trameV2(&cchecksum);
    }
    return -1;
    break;

  case COMIO2_CMD_CALLFUNCTION: 
    if(l_data < 1 || data[0]>=COMIO2_MAX_FX) // il faut un code fonction valide
    {
      if(id)
        _comio_send_errorV2(id, COMIO2_ERR_PARAMS);
      return 0;
    }
    if(comio_functionsV2[data[0]])
    {
      int retval=comio_functionsV2[data[0]](id, &(data[1]), l_data-1, userdata);
      if(id)
      {
        uint8_t r=0;
        
        _comio_debut_trameV2(id, COMIO2_CMD_CALLFUNCTION, 2, &cchecksum);
        r=retval & 0xFF;
        cchecksum+=r;
        writeF(r);
        r=(retval >> 8) & 0xFF;
        cchecksum+=r;
        writeF(r);
        _comio_fin_trameV2(&cchecksum);
      }
    }
    else
    {
      if(id)
        _comio_send_errorV2(id, COMIO2_ERR_UNKNOWN_FUNC);
      return 0;
    }
    return -1;
    break;

  default:
    return 0;
  }
}


void Comio2::sendTrap(unsigned char num_trap, char *data, char l_data)
/**
 * \brief     Ã©mission d'une trame TRAP de type "Comio2" dans le buffer de sortie.
 * \details   Construit une trame de TRAP et l'envoie dans le buffer de sortie
 * \param     num_trap  numÃ©ro du trap
 * \param     value     donnÃ©es d'accompagnement du trap
 * \param     l_value   nombre de donnÃ©es dans le champ valeur
 */
{
  int cchecksum=0;

  _comio_debut_trameV2(COMIO2_TRAP_ID, num_trap, l_data, &cchecksum);
  for(int i=0;i<l_data;i++)
  {
    cchecksum+=data[i];
    writeF(data[i]);
  }
  _comio_fin_trameV2(&cchecksum);
}


int Comio2::setMemory(unsigned int addr, unsigned char value)
/**
 * \brief     Ecriture d'une case mÃ©moire.
 * \param     addr  adresse de la case Ã  mettre Ã  jour
 * \return    0 si OK ou -1 si addr hors scope
 */
{
  if(addr >= 0 || addr <  COMIO_MAX_M)
  {
    comio_memory[addr]=value;
    return 0;
  }
  return -1;
}


int Comio2::getMemory(unsigned int addr)
/**
 * \brief     Lecture d'une case mÃ©moire.
 * \param     addr  adresse de la case Ã  lire
 * \return    la valeur de la case ou -1 si addr hors scope
 */
{
  if(addr >= 0 || addr <  COMIO_MAX_M)
  {
    return (int)comio_memory[addr];
  }
  return -1;
}


void Comio2::setFunction(unsigned char num_function, callback2_f function)
/**
 * \brief     Association d'une fonction Ã  un "port" (numÃ©ro de fonction).
 * \details  
 * \param     num_function  numÃ©ro de fonction
 * \param     function      fonction associÃ©e au numÃ©ro / mettre NULL pour desalouer une fonction
 */
{
  if(num_function<COMIO2_MAX_FX)
    comio_functionsV2[num_function]=function;
}


//
//
// Commun V1 et V2
//
//
void Comio2::init(void)
/**
 * \brief     Initialisation de COMIO1 sans paramÃ¨tre.
 */
{
#ifdef _COMIO1_COMPATIBLITY_MODE_
  init(NULL);
#endif
}


inline int _comio2_serial_read()
/**
 * \brief     fonction de lecture d'un caractÃ¨re utilisÃ©e par dÃ©faut.
 * \details   Point de lecture pour la rÃ©ception des commandes/donnÃ©es Ã  traiter par Comio si aucune autre fonction n'a Ã©tÃ© positionnÃ©e par la methode setReadFunction. Lit le premier caractÃ¨re du buffer d'entrÃ©e
 * \return    < 0 (-1) si pas de donnÃ©es disponible, le caractÃ¨re lu sinon.
 */
{
  return Serial.read();
}


inline int _comio2_serial_write(char car)
/**
 * \brief     fonction d'Ã©criture par dÃ©faut.
 * \param     car   caractÃ¨re a insÃ©rer dans le buffer de sorie
 * \details   Point d'Ã©criture des rÃ©sultats renvoyÃ©s par Comio si aucune autre fonction n'a Ã©tÃ© positionnÃ©e par la methode setWriteFunction. Ajoute un caractere dans le buffer de sortie
 * \return    toujours 0
 */
{
  Serial.write(car);
  return 0;
}


inline int _comio2_serial_available()
/**
 * \brief     fonction permettant de connaitre le nombre de caractÃ¨res prÃ©sent dans le buffer d'entrÃ©e.
 * \details   fonction par defaut si aucune autre fonction n'a Ã©tÃ© positionnÃ©e par la methode setAvailableFunction.
 * \return    nombre de caractÃ¨res disponibles
 */
{
  return Serial.available();
}


inline int _comio2_serial_flush()
/**
 * \brief     fonction permettant d'attendre le "vidage" du buffer de sortie.
 * \details   fonction par defaut si aucune autre fonction n'a Ã©tÃ© positionnÃ©e par la methode setFlushFunction.
 * \return    toujours 0
 */
{
  Serial.flush();
  return 0;
}


Comio2::Comio2()
/**
 * \brief     constructeur classe Comio2.
 * \details   initialise les donnÃ©es des instances Comio2.
 */
{
#ifdef _COMIO1_COMPATIBLITY_MODE_
  memset(comio_digitals,-1,sizeof(comio_digitals));
  memset(comio_analogs,-1,sizeof(comio_analogs));
  memset(comio_functions,0,sizeof(comio_functions));
#endif
  memset(comio_functionsV2,0,sizeof(comio_functionsV2));
  memset(comio_memory,0,sizeof(comio_memory));

  writeF=_comio2_serial_write;
  readF=_comio2_serial_read;
  availableF=_comio2_serial_available;
  flushF=_comio2_serial_flush;

  userdata = NULL;
  
  
}


/**
 */
int Comio2::run()
/**
 * \brief     fait "tourner" COMIO.
 * \details   Fonction Ã  appeler le plus souvant possible pour pouvoir prendre en charge les demandes en provenance d'un client et les traiter.
 * \return    0 = pas de trames traitÃ©es ou donnÃ©es incorrectes / <> 0 code de traitement (> 0 Comio1 < 0 Comio2).
 */
{
  if(!availableF())
    return 0;

  unsigned char cptr=0;
  boolean flag;
#ifdef _COMIO1_COMPATIBLITY_MODE_
  boolean new_frame=true;
#endif

  flag=false;
  while(availableF() && cptr<10) // lecture jusqu'Ã  10 caractÃ¨res pour dÃ©tecter un dÃ©but de trame
  {
    int c=readF();
#ifdef _COMIO1_COMPATIBLITY_MODE_
    if(c=='?') // un dÃ©but de trame "type 1" est dÃ©tectÃ©
    {
      flag=true;
      new_frame=false;
      break;
    }
#endif
    if(c=='[') // nouveau format de trame
    {
      flag=true;
      break;
    }
    cptr++;
  }
  if(!flag)
    return 0; // pas de dÃ©but de trame, on rend la main pour ne pas trop bloquer la boucle principale

#ifdef _COMIO1_COMPATIBLITY_MODE_
  if(new_frame)               // format de trame COMIO2
  {
#endif
    char data[COMIO2_MAX_DATA];
    unsigned char id;
    unsigned char cmd;
    int l_data;
    int ret=_comio_read_frameV2(&id, &cmd, data, &l_data);
    if(ret)
      ret=_comio_do_operationV2(id, cmd, data, l_data);
#ifdef _COMIO1_COMPATIBLITY_MODE_
    return ret;
  }
  else
  {           
    unsigned char op, var, type;
    unsigned int val;
    if(_comio_read_frame(&op,&var,&type,&val)) // si on obtient pas une trame on rend la main "sans rien dire"
    {
      int ret=_comio_valid_operation(op,var,type,val);
      if(!ret)
        ret=_comio_do_operation(op,var,type,val);
      else
      {
        _comio_send_error_frame(ret);
        return 0;
      }
      return ret;
    }
  }
#endif
  return 0;

