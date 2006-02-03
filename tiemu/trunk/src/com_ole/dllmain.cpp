/*  OLE Automation registering for TiEmu
 *
 *  Copyright (c) 2006 Romain Li�vin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "oleaut.h"	// G�n�r� automatiquement par MIDL
#include "registry.h"		// Enregistre composant dans base de registres
#include "tiemuole.h"

HINSTANCE g_hInstance;		// Handler d'instance du processus

//* DllCanUnloadNow() *******************************************************
//*
//* Descriptif : Indique si la DLL peut �tre d�charg�e de la m�moire
//*
//* Param�tres en entr�e : aucun
//*
//* Param�tres en sortie : code de retour HRESULT
//*   S_OK    : La DLL peut �tre d�charg�e de la m�moire
//*   S_FALSE : La DLL est encore utilis�e ou v�rouill�e
//*
//***************************************************************************
HRESULT __stdcall DllCanUnloadNow()
{	
	/*
	if(g_cServerLocks == 0 && g_cComponents == 0)
		return S_OK;
	else
		return S_FALSE;
	*/
	return S_OK;
}

//* DllGetClassObject() *****************************************************
//*
//* Descriptif : Instancie une classe CLSID et retourne une interface IID
//*              sur l'objet cr�e.
//*
//* Param�tres en entr�e :
//*   clsid   : Identificateur d'une classe COM (CLSID_) � instancier.
//*             ( Rq : la seule reconnue est CLSID_MonitorARINC )
//*   iid     : Identificateur d'une interface COM (IID_) 
//*             ( Rq : les interfaces IID_IUnknown et IID_IClassFactory
//*             sont les seules reconnues. )
//*
//* Param�tres en sortie :
//*   **ppv   : Pointeur sur l'interface IID d'une classe CLSID
//*             instanci�e.
//*
//*   retour  : code HRESULT
//*      CLASS_E_CLASSNOTAVAILABLE  
//*                    : Classe CLSID non prise en charge
//*      E_OUTOFMEMORY : M�moire insuffisante pour cr�e la classe fabrique
//*      E_NOINTERFACE : Interface IID non prise en charge
//*      S_OK          : Pas d'erreur
//*
//***************************************************************************
HRESULT __stdcall DllGetClassObject(REFCLSID clsid, REFIID iid, void** ppv)
{
	MessageBox(NULL, "Info", "DllGetClassObject", MB_OK);

	// V�rification que la classe est bien prise en compte
	if(clsid != CLSID_TiEmuOLE)
	{
		MessageBox(NULL, "Error", "DllGetClassObject", MB_OK);
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	// Cr�taion d'une fabrique de classe
	TiEmuOLE* ole_interface = new TiEmuOLE;
	if(ole_interface == NULL)
	{
		MessageBox(NULL, "Error", "DllGetClassObject", MB_OK);
		return E_OUTOFMEMORY;
	}

	// QueryInterface pour demander probablement IClassFactory
	HRESULT hr = ole_interface->QueryInterface(iid, ppv);
	ole_interface->Release();
	if(FAILED(hr))
	{
		MessageBox(NULL, "Error", "DllGetClassObject", MB_OK);
		return hr;
	}

	return S_OK;
}

//* DllRegisterServer() *****************************************************
//*
//* Descriptif : Enregistre la DLL et les informations de types dans la base
//*              des registres.
//*
//* Param�tres en entr�e : aucun
//*
//* Param�tres en sortie : code de retour HRESULT
//*   S_OK : Pas d'erreur
//*
//***************************************************************************
HRESULT __stdcall DllRegisterServer()
{
	MessageBox(NULL, "Info", "DllRegisterServer", MB_OK);

	// Chemin d'acc�s de la DLL
	char cDllPath[512];			// Cha�ne ANSI
	OLECHAR wDllPath[512];		// Cha�ne OLE

	// On r�cup�re le nom de fichier du pr�sent module
	GetModuleFileName(g_hInstance, cDllPath, 500);

	// Conversion en cha�ne OLE
	mbstowcs(wDllPath, cDllPath, 500);

	// Chargement des informations de type contenus dans la DLL (� modifier ?)
	ITypeLib* pTypeLib;
	HRESULT hr = LoadTypeLibEx(wDllPath, REGKIND_REGISTER, &pTypeLib);
	if(FAILED(hr))
	{
		MessageBox(NULL, "Error", "DllRegisterServer", MB_OK);
		return hr;
	}
	pTypeLib->Release();

	// Enregistrement de la DLL dans la base des registres
	hr = RegisterServer("TiEmu.exe", CLSID_TiEmuOLE, 
		"TiEmu OLE Interface", "TiEmu.TiEmu", "TiEmu.TiEmu.1", NULL);
	if(FAILED(hr))
	{
		MessageBox(NULL, "Error", "DllRegisterServer", MB_OK);
		return hr;
	}

	return S_OK;
}

//* DllUnRegisterServer() ***************************************************
//*
//* Descriptif : Retire les informations concernant la DLL et les 
//*              informations de type de la base des registres.
//*
//* Param�tres en entr�e : aucun
//*
//* Param�tres en sortie : code de retour HRESULT
//*   S_OK : Pas d'erreur
//*
//***************************************************************************
HRESULT __stdcall DllUnregisterServer()
{
	UnRegisterTypeLib(LIBID_TiEmuOLELib, 1, 0, LANG_NEUTRAL, SYS_WIN32);
	return UnregisterServer(CLSID_TiEmuOLE, "TiEmu.TiEmu", "TiEmu.TiEmu.1");
}

//* DllMain() ***************************************************************
//*
//* Descriptif : Proc�dure principale pour une DLL �quivalent � Main()
//*
//* Param�tres en entr�e : 
//*   hInstance : Handler d'instance du processus d'ex�cution
//*   dwReason  : Param�tre syst�me non utilis�
//*   pv*       : Param�tre syst�me non utilis�
//*
//* Param�tres en sortie : 
//*   retour BOOL : Toujours TRUE
//*
//***************************************************************************
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, void* pv)
{
	g_hInstance = hInstance;
	return TRUE;
}
