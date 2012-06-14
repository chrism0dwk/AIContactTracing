/* ./src/data/occultReader.cpp
 *
 * Copyright 2012 Chris Jewell <chrism0dwk@gmail.com>
 *
 * This file is part of InFER.
 *
 * InFER is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * InFER is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InFER.  If not, see <http://www.gnu.org/licenses/>. 
 */

// Occult read/write classes

#include "occultReader.h"

#include <stdexcept>
#include <exception>
#include <string>
#include <cstdlib>
#include <string.h>

#include "stlStrTok.hpp"

OccultReader::OccultReader() : rowPos(0),posteriorSize(0)
{
  // ctor
}



OccultReader::~OccultReader()
{
  // Closes the file if we need it
  if (file.is_open()) file.close();
}



void OccultReader::open(const char* const filename, const int skip)
{

  string buffer;
  
  cout << "Opening occults and calculating posterior size: " << filename << "..." << flush;

  file.open(filename,ios::in);
  if (!file.is_open())
    {
      throw ios_base::failure("OccultReader::open failed.  Check filename/permissions");
    }
  
  cout << "Start pos: " << file.tellg();
  
  // Calculate number of rows
  while(!file.eof()) {
    getline(file,buffer);
    if(buffer.size() == 0) {
      cerr << "Terminating count at newline" << endl;
      break;
    }
    posteriorSize++;
  }
  

  // Clear EOF bit
  file.clear();
  
  // Rewind file
  file.seekg(0);
  
  cout << "Done with " << posteriorSize << " lines" << endl;

}



map<size_t,double> OccultReader::fetch(const int& row)
{
  // Fetches a row of occults returning them in a map.

  string linebuffer;
  vector<string> tupleRow;
  vector<string> tupleData;
  vector<string>::iterator currentRow;
  pair<size_t,double> tuple;
  map<size_t,double> occults;
  char errmsg[200];

  try
    {
      // Raise an exception if the requested row is out of bounds
      if(row >= posteriorSize) {
	sprintf(errmsg,"Requested row (%i) out of bounds",row);
	throw range_error(errmsg);
      }
      
      // Rewind if requested row is less than current position
      if (row < rowPos) rewind();
      
      // Wind forward in the file to row
      // and read in row
      while (rowPos < row)
        {
          getline(file,linebuffer);
          rowPos++;
        }
      
      getline(file,linebuffer);
      rowPos++;
      if (linebuffer.size() == 0) throw range_error("EOF or blank line encountered");

      stlStrTok(tupleRow,linebuffer," ");

      // Remove dud tuple from end of row
      if (tupleRow.back().size() == 0)
        {
          tupleRow.pop_back();
        }

      // Loop over the vector of tokens and split the label:infection-time pairs
      currentRow = tupleRow.begin();
      while (currentRow != tupleRow.end())
        {

          stlStrTok(tupleData,*currentRow,":");
          if (tupleData.size() != 2)
            {
              sprintf(errmsg,"Abnormal tuple at line %i", rowPos);
              throw runtime_error(errmsg);
            }

          tuple.first = atoi(tupleData[0].c_str()); // label
          tuple.second = atof(tupleData[1].c_str()); // Infection time

          occults.insert(tuple); // Add to our set
          currentRow++;
        }
    }
  catch (exception& e)
    {
      cerr << "Exception in OccultReader::fetch: " << e.what() << "\n";
      throw e;
    }
    
  return occults;
}



size_t OccultReader::size()
{
  // Gives the length of the occult dataset
  return posteriorSize;
}



void OccultReader::rewind()
{
  // Rewinds the file pointer to the beginning
  try
    {
      file.seekg(0);
      rowPos = 0;
    }
  catch (exception& e)
    {
      cerr << "Exception rewinding occult file: " << e.what() << "\n";
    }
}
