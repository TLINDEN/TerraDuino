// DB.cpp
/*
  Database library for Arduino 
  Written by Madhusudana das
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Arduino.h"
#include "DB.h"

/**************************************************/
// private functions
int DB::writeHead()
{
    byte * p = (byte*)(void*)&DB_head;
	int ee = DB_head_ptr;
    int i;
    for (i = 0; i < (int)sizeof(DB_head); i++)
      EEPROM.write(ee++, *p++);
    return i;
}

int DB::readHead()
{
    byte* p = (byte*)(void*)&DB_head;
	int ee = DB_head_ptr;
    int i;
    for (i = 0; i < (int)sizeof(DB_head); i++)
      *p++ = EEPROM.read(ee++);
    return i;
}

int DB::EEPROM_dbWrite(int ee, const byte* p)
{
    int i;
    for (i = 0; i < DB_head.rec_size; i++)
      EEPROM.write(ee++, *p++);
    return i;
}

int DB::EEPROM_dbRead(int ee, byte* p)
{  
    int i;
    for (i = 0; i < DB_head.rec_size; i++)
      *p++ = EEPROM.read(ee++);
    return i;
}

/**************************************************/
// public functions

void DB::create(int head_ptr, byte recsize)
{
  DB_head_ptr = head_ptr;
  DB_head.n_recs   = 0;
  DB_head.rec_size = recsize;
  writeHead();
}

void DB::open(int head_ptr)
{
  DB_head_ptr = head_ptr;
  DB_tbl_ptr  = head_ptr + DB_HEAD_SIZE;
  readHead();
}
//other operations commit DB_head edits to EEPROM so no need for a DB_close


boolean DB::write(byte recno, const DB_Rec rec)
{
  DB_error = DB_OK;
  if (recno>0 && recno<=DB_head.n_recs+1)
    EEPROM_dbWrite(DB_tbl_ptr+((recno-1)*DB_head.rec_size), rec);
  else
    DB_error = DB_RECNO_OUT_OF_RANGE;
  return DB_error==DB_OK;
}


boolean DB::read(byte recno, DB_Rec rec)
{
  DB_error = DB_OK;
  if (recno>0 && recno<=DB_head.n_recs)
    EEPROM_dbRead(DB_tbl_ptr+((recno-1)*DB_head.rec_size), rec);
  else
    DB_error = DB_RECNO_OUT_OF_RANGE;
  return DB_error==DB_OK;
}


boolean DB::deleteRec(byte recno)
{
  DB_error = DB_OK;
  if (recno<0 || recno>DB_head.n_recs)
  {  Serial.println("recno out of range");
    DB_error = DB_RECNO_OUT_OF_RANGE;
    return false;
  }
  DB_Rec rec = (byte*)malloc(DB_head.rec_size);
  for (int i=recno+1; i<=DB_head.n_recs; i++)
  {
    read(i,rec);
    write(i-1,rec);
  }  
  free(rec);
  DB_head.n_recs--;
  EEPROM.write(DB_head_ptr,DB_head.n_recs);
  return true;
}


boolean DB::insert(byte recno, DB_Rec rec)
{
  DB_error = DB_OK;
  if (recno<0 || recno>DB_head.n_recs)
  {  Serial.println("recno out of range");
    DB_error = DB_RECNO_OUT_OF_RANGE;
    return false;
  }
  DB_Rec buf = (byte*)malloc(DB_head.rec_size);
  for (int i=DB_head.n_recs; i>=recno; i--)
  {
    read(i,buf);
    write(i+1,buf);
  }
  free(buf);
  write(recno,rec);  
  DB_head.n_recs++;
  EEPROM.write(DB_head_ptr,DB_head.n_recs);
  return true;
}

void DB::append(DB_Rec rec)
{
  DB_error = DB_OK;
  DB_head.n_recs++;
  write(DB_head.n_recs,rec);
  EEPROM.write(DB_head_ptr,DB_head.n_recs);
}

byte DB::nRecs()
{
  return DB_head.n_recs;
}
