// MDF4_SimpleReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ImportMdf4.h"
#include "mdf4.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[])
{
	CMdf4FileImport *pImport = new CMdf4FileImport;

  if(argc != 2)
  {
    printf("Error: Wrong number of arguments\n");
    return 1;
  }
  
  if (pImport->MyImportFile(argv[1]))
	{
	}

	return 0;
}

