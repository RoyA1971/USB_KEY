#include <vcl.h>
#include <System.IOUtils.hpp>
#include <windows.h> // Include Windows API header
#include <winioctl.h> // Inclure winioctl.h pour DISK_GEOMETRY
#include <setupapi.h>
#include <aclapi.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <initguid.h>
#include <usbioctl.h>
#include <iostream>
#pragma hdrstop

#include "Main.h" // Assurez-vous que le fichier d'en-tête Unit1.h est inclus ici

#pragma package(smart_init)
#pragma resource "*.dfm"
#pragma comment(lib, "setupapi.lib")
TUSB_RAW *USB_RAW;

// Define IOCTL_USB_CYCLE_PORT
#define IOCTL_USB_CYCLE_PORT CTL_CODE(FILE_DEVICE_USB, 0x009, METHOD_BUFFERED, FILE_ANY_ACCESS)

__fastcall TUSB_RAW::TUSB_RAW(TComponent* Owner)
	: TForm(Owner)
{
}

// Déclaration externe de hEdit2 (assurez-vous qu'elle est accessible)

bool Interrompre = false; // Variable globale pour signaler l'interruption
bool Quitter = false; // Variable globale pour signaler l'interruption

bool DisableAutoRun(TUSB_RAW *Form)
{
	HKEY hKey;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 0, KEY_SET_VALUE, &hKey);

		Form->Edit1->Text = "";
		Form->Edit2->Text = "";
		Form->Edit3->Text = "";
		Form->Edit4->Text = "";
		Form->Edit5->Text = "";
		Form->Edit6->Text = "";
		Form->Edit7->Text = "";
		Form->Edit8->Text = "";
		Form->Edit9->Text = "";
		Form->Edit10->Text = "";
		Form->Edit11->Text = "";
		Form->Edit12->Text = "";
		Form->Edit13->Text = "";


	if (result == ERROR_SUCCESS) {
		Form->Edit1->Text = " Registry key opened successfully! ";
		Form->Edit1->Text = " Registry key opened ? : " + Form->Edit1->Text;

		DWORD value = 0xFF; // Désactiver AutoRun pour tous les lecteurs
		result = RegSetValueEx(hKey, L"NoDriveTypeAutoRun", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

		if (result == ERROR_SUCCESS) {
			Form->Edit2->Text = " Registry value set successfully! ";
			Form->Edit2->Text = " Registry value set ? : " + Form->Edit2->Text;
		} else {
			wchar_t errorMsg[256];
			swprintf(errorMsg, 256, L"Erreur lors de la configuration de la valeur de registre. Code d'erreur: %d", result);
			Form->Edit2->Text = " Registry value set ? : ";
			Form->Edit2->Text = Form->Edit2->Text + errorMsg;
			RegCloseKey(hKey);
			return false;
		}

		// Fermer la clé de registre
		RegCloseKey(hKey);
		Form->Edit3->Text = " AutoRun désactivé avec succès. ";
		Form->Edit3->Text = " AutoRun désactivé ? : " + Form->Edit3->Text;
		return true;
	} else {
		wchar_t errorMsg[256];
		swprintf(errorMsg, 256, L" Because : Failed to open registry key. Error code: %d", result);
		Form->Edit3->Text = " Have You the Amdinistrator Rights ? | ";
		Form->Edit3->Text = Form->Edit3->Text + errorMsg;
		return false;
	}
}

bool EnableAutoRun(TUSB_RAW *Form)
{
	HKEY hKey;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 0, KEY_SET_VALUE, &hKey);

		Form->Edit1->Text = "";
		Form->Edit2->Text = "";
		Form->Edit3->Text = "";
		Form->Edit4->Text = "";
		Form->Edit5->Text = "";
		Form->Edit6->Text = "";
		Form->Edit7->Text = "";
		Form->Edit8->Text = "";
		Form->Edit9->Text = "";
		Form->Edit10->Text = "";
		Form->Edit11->Text = "";
		Form->Edit12->Text = "";
		Form->Edit13->Text = "";


	if (result == ERROR_SUCCESS) {
		Form->Edit5->Text = " Registry key opened successfully! ";
		Form->Edit5->Text = " Registry key opened ? : " + Form->Edit5->Text;

		DWORD value = 0x91; // Activer AutoRun pour la plupart des types de lecteurs
		result = RegSetValueEx(hKey, L"NoDriveTypeAutoRun", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

		if (result == ERROR_SUCCESS) {
			Form->Edit6->Text = L" | Registry value set successfully! ";
			Form->Edit6->Text = " Registry value set ? : " + Form->Edit6->Text;

		} else {
			wchar_t errorMsg[256];
			swprintf(errorMsg, 256, L"Erreur lors de la configuration de la valeur de registre. Code d'erreur: %d", result);
			Form->Edit6->Text = L" Registry value set ? : ";
			Form->Edit6->Text = Form->Edit6->Text + errorMsg;
			RegCloseKey(hKey);
			return false;
		}

		// Fermer la clé de registre
		RegCloseKey(hKey);
		Form->Edit7->Text = " AutoRun désactivé ? : ";
		Form->Edit7->Text = Form->Edit7->Text + L"AutoRun activé avec succès. ";
		return true;
	} else {
		wchar_t errorMsg[256];
		swprintf(errorMsg, 256, L"Failed to open registry key. Error code: %d", result);
		Form->Edit7->Text = " Have You the Amdinistrator Rights ? | ";
		Form->Edit7->Text = Form->Edit7->Text + errorMsg;
		return false;

	}
}

bool CycleUSBPort(const GUID* guid)
{
	HDEVINFO deviceInfoSet = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (deviceInfoSet == INVALID_HANDLE_VALUE)
	{
		ShowMessage(L"Echec de récupération des infos de réglages du device.");
		return false;
	}

	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	// Declare a variable of type SP_DEVINFO_DATA
	SP_DEVINFO_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	for (DWORD i = 0; SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, guid, i, &deviceInterfaceData); ++i)
	{
		DWORD requiredSize = 0;
		SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

		SP_DEVICE_INTERFACE_DETAIL_DATA* deviceInterfaceDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA*)malloc(requiredSize);
		if (!deviceInterfaceDetailData)
		{
			ShowMessage(L"Echec dans l'allocation de mémoire.");
			continue;
		}

		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, requiredSize, &requiredSize, &deviceInfoData))
		{
			ShowMessage(L"Echec de récupération des infos de réglages du device.");
			free(deviceInterfaceDetailData);
			continue;
		}

		HANDLE deviceHandle = CreateFile(deviceInterfaceDetailData->DevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (deviceHandle == INVALID_HANDLE_VALUE)
		{
			ShowMessage(L"Echec d'ouverture du device.");
			free(deviceInterfaceDetailData);
			continue;
		}

		free(deviceInterfaceDetailData);

		DWORD bytesReturned = 0;
		if (!DeviceIoControl(deviceHandle, IOCTL_USB_CYCLE_PORT, NULL, 0, NULL, 0, &bytesReturned, NULL))
		{
			ShowMessage(L"Balayage des USB port des devices a échoué.");
			CloseHandle(deviceHandle);
			continue;
		}

		CloseHandle(deviceHandle);
		SetupDiDestroyDeviceInfoList(deviceInfoSet);
		return true;
	}

	SetupDiDestroyDeviceInfoList(deviceInfoSet);
	ShowMessage(L"Ne trouve pas l'USB device.");
	return false;
}

bool RestartUSBDevice()
{
	HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (deviceInfoSet == INVALID_HANDLE_VALUE)
	{
		ShowMessage(L"Echec de récupération des infos de réglages du device.");
		return false;
	}

	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	SP_DEVINFO_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	for (DWORD i = 0; SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &GUID_DEVINTERFACE_USB_DEVICE, i, &deviceInterfaceData); ++i)
	{
		DWORD requiredSize = 0;
		SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

		SP_DEVICE_INTERFACE_DETAIL_DATA* deviceInterfaceDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA*)malloc(requiredSize);
		if (!deviceInterfaceDetailData)
		{
			ShowMessage(L"Echec dans l'allocation de mémoire.");
            free(deviceInterfaceDetailData);
			continue;
		}

		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, requiredSize, &requiredSize, &deviceInfoData))
		{
			 ShowMessage(L"N'a pas pu récupérer le détail de l'interface du device.");
			free(deviceInterfaceDetailData);
			continue;
		}

		HANDLE deviceHandle = CreateFile(deviceInterfaceDetailData->DevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (deviceHandle == INVALID_HANDLE_VALUE)
		{
			 ShowMessage(L"Echec de tentative d'ouverture du device.");
			free(deviceInterfaceDetailData);
			continue;
		}

		free(deviceInterfaceDetailData);

		// Retry eject and re-enumeration up to 3 times
		for (int attempt = 0; attempt < 3; ++attempt)
		{
			CONFIGRET cr = CM_Request_Device_Eject(deviceInfoData.DevInst, NULL, NULL, 0, 0);
			if (cr != CR_SUCCESS)
			{
				UnicodeString message = L"Echec de la tentative d'éjection du device. Tentative: " + UnicodeString(attempt + 1);
                ShowMessage(message);

				continue;
			}

			Sleep(50);

			cr = CM_Reenumerate_DevNode(deviceInfoData.DevInst, 0);
			if (cr != CR_SUCCESS)
			{
				UnicodeString message = L"Echec de tentative de l'énumération des devices." + UnicodeString(attempt + 1);
				ShowMessage(message);

				continue;
			}

			// Successfully restarted the device
				UnicodeString message = L"Device redémarré avec succès avec cette tentative " + UnicodeString(attempt + 1);
				ShowMessage(message);

			CloseHandle(deviceHandle);
			SetupDiDestroyDeviceInfoList(deviceInfoSet);
			return true;
		}

		CloseHandle(deviceHandle);
	}

	SetupDiDestroyDeviceInfoList(deviceInfoSet);

	ShowMessage(L"Le programme ne trouve pas l'USB device.");
	return false;
}

bool EnablePrivilege(LPCWSTR privilege)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return false;
	}

	if (!LookupPrivilegeValue(NULL, privilege, &luid))
	{
		CloseHandle(hToken);
		return false;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{
		CloseHandle(hToken);
		return false;
	}

	CloseHandle(hToken);
	return true;
}

bool UnlockUsbDrive(const wchar_t* driveLetter, TUSB_RAW *Form)
{
    int selectedIndex = Form->LstBx_Usb_Keys_List->ItemIndex;
    if (selectedIndex != -1)
    {
        // Activer le privilège SE_TAKE_OWNERSHIP_NAME
        if (!EnablePrivilege(SE_TAKE_OWNERSHIP_NAME))
        {
            Form->Edit9->Text = L"Failed to enable privilege.";
            return false;
        }

        // Construire le chemin complet de l'USB
        wchar_t drivePath[7] = { L'\\', L'\\', L'.', L'\\', driveLetter[0], L':', L'\0' };

        // Créer un SID pour tous les utilisateurs
        PSID pEveryoneSID = NULL;
        SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
        if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
        {
            Form->Edit9->Text = (L"Failed to initialize SID. Error code: " + std::to_wstring(GetLastError())).c_str();
            return false;
        }

        // Créer une ACE (Access Control Entry) qui permet tout accès
        EXPLICIT_ACCESS ea;
        ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
        ea.grfAccessPermissions = GENERIC_ALL;
        ea.grfAccessMode = GRANT_ACCESS;
        ea.grfInheritance = NO_INHERITANCE;
        ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea.Trustee.ptstrName = (LPWSTR)pEveryoneSID;

        // Créer une nouvelle ACL (Access Control List) qui contient l'ACE
        PACL pACL = NULL;
        DWORD dwRes = SetEntriesInAcl(1, &ea, NULL, &pACL);
        if (ERROR_SUCCESS != dwRes)
        {
            Form->Edit10->Text = (L"Failed to set entries in ACL. Error code: " + std::to_wstring(dwRes)).c_str();
            FreeSid(pEveryoneSID);
            return false;
        }

        // Appliquer l'ACL à l'objet de sécurité du périphérique USB
        dwRes = SetNamedSecurityInfo((LPWSTR)drivePath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL);
        if (ERROR_SUCCESS != dwRes)
        {
            Form->Edit11->Text = (L"Failed to set security info. Error code: " + std::to_wstring(dwRes)).c_str();
            FreeSid(pEveryoneSID);
            LocalFree(pACL);
            return false;
        }

        // Libérer les ressources
        FreeSid(pEveryoneSID);
        LocalFree(pACL);

        Form->Edit12->Text = L"USB drive unlocked successfully!";
        Form->LstBx_Usb_Key_UnLocked->Items->Strings[selectedIndex] = "UNLOCKED !";
        Form->LstBx_Usb_Key_Locked->Items->Strings[selectedIndex] = " ";
        Form->LstBx_Usb_Keys_Drive_Letter->ItemIndex = selectedIndex;
        Form->LstBx_Usb_Key_Status->ItemIndex = selectedIndex;
        Form->LstBx_Usb_Key_UnLocked->ItemIndex = selectedIndex;
        Form->ListBoxCopy->ItemIndex = selectedIndex;
        Form->LstBx_Usb_Key_Locked->ItemIndex = selectedIndex;

        return true;
    }
    else
    {
        ShowMessage(L"Aucune Lettre de lecteur sélectionnée.");
        return false;
    }
}


bool LockUsbDrive(const wchar_t* driveLetter, TUSB_RAW *Form)
{
	int selectedIndex = Form->LstBx_Usb_Keys_List->ItemIndex;
	if (selectedIndex != -1)
	{
		// Activer le privilège SE_TAKE_OWNERSHIP_NAME
		if (!EnablePrivilege(SE_TAKE_OWNERSHIP_NAME))
		{
			Form->Edit9->Text = L"Failed to enable privilege.";
			return false;
		}

        // Construire le chemin complet de l'USB
		wchar_t drivePath[7] = { L'\\', L'\\', L'.', L'\\', driveLetter[0], L':', L'\0' };

        // Créer un SID pour tous les utilisateurs
		PSID pEveryoneSID = NULL;
        SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
		if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
        {
            Form->Edit9->Text = (L"Failed to initialize SID. Error code: " + std::to_wstring(GetLastError())).c_str();
            return false;
		}

        // Créer une ACE (Access Control Entry) qui refuse tout accès
        EXPLICIT_ACCESS ea;
        ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
		ea.grfAccessPermissions = GENERIC_ALL;
        ea.grfAccessMode = DENY_ACCESS;
        ea.grfInheritance = NO_INHERITANCE;
        ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea.Trustee.ptstrName = (LPWSTR)pEveryoneSID;

        // Créer une nouvelle ACL (Access Control List) qui contient l'ACE
		PACL pACL = NULL;
        DWORD dwRes = SetEntriesInAcl(1, &ea, NULL, &pACL);
        if (ERROR_SUCCESS != dwRes)
        {
			Form->Edit10->Text = (L"Failed to set entries in ACL. Error code: " + std::to_wstring(dwRes)).c_str();
            FreeSid(pEveryoneSID);
            return false;
        }

        // Appliquer l'ACL à l'objet de sécurité du périphérique USB
        dwRes = SetNamedSecurityInfo((LPWSTR)drivePath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL);
        if (ERROR_SUCCESS != dwRes)
		{
            Form->Edit11->Text = (L"Failed to set security info. Error code: " + std::to_wstring(dwRes)).c_str();
            FreeSid(pEveryoneSID);
            LocalFree(pACL);
			return false;
        }

		// Libérer les ressources
        FreeSid(pEveryoneSID);
        LocalFree(pACL);

        Form->Edit12->Text = L"USB drive locked successfully!";
		Form->LstBx_Usb_Key_Locked->Items->Strings[selectedIndex] = "LOCKED !";
		Form->LstBx_Usb_Key_UnLocked->Items->Strings[selectedIndex] = " ";
		Form->LstBx_Usb_Keys_Drive_Letter->ItemIndex = selectedIndex;
		Form->LstBx_Usb_Key_Status->ItemIndex = selectedIndex;
		Form->LstBx_Usb_Key_UnLocked->ItemIndex = selectedIndex;
		Form->ListBoxCopy->ItemIndex = selectedIndex;
		Form->LstBx_Usb_Key_Locked->ItemIndex = selectedIndex;

		return true;
	}
    else
    {
		ShowMessage(L"Aucune Lettre de lecteur sélectionnée.");
		return false;
	}
}

void __fastcall TUSB_RAW::Btn_Usb_Keys_ShowClick(TObject *Sender)
{
	LstBx_Usb_Keys_List->Items->Clear();
	ListBoxCopy->Items->Clear();
	LstBx_Usb_Keys_Drive_Letter->Items->Clear();
	LstBx_Usb_Key_Locked->Items->Clear();
	LstBx_Usb_Key_UnLocked->Items->Clear();
	LstBx_Usb_Key_Status->Items->Clear();

	Btn_Usb_Keys_Show->Visible = false;
	Btn_Annuler_Retour->Visible = true;

	wchar_t Drive = L'A';
	wchar_t DriveStr[4] = { Drive, L':', L'\\', L'\0' };
	for (int i = 0; i < 26; i++, Drive++)
	{
		DriveStr[0] = Drive;
		if (TDirectory::Exists(DriveStr) || GetDriveType(DriveStr) == DRIVE_REMOVABLE)
		{
			UINT driveType = GetDriveType(DriveStr);
			if (driveType == DRIVE_REMOVABLE || driveType == DRIVE_UNKNOWN)
			{
				wchar_t volumeName[MAX_PATH] = L"";
				if (GetVolumeInformation(DriveStr, volumeName, MAX_PATH, NULL, NULL, NULL, NULL, 0))
				{
					LstBx_Usb_Keys_List->Items->Add(System::String(DriveStr[0]) + String(": ") + String(volumeName));
					ListBoxCopy->Items->Add(System::String(DriveStr[0]) + String(": ") + String(volumeName));
					LstBx_Usb_Keys_Drive_Letter->Items->Add(DriveStr[0]);
					LstBx_Usb_Key_Locked->Items->Add(String(" "));
					LstBx_Usb_Key_UnLocked->Items->Add(L"UNLOCKED !");
					LstBx_Usb_Key_Status->Items->Add(L"...");
				}
				else {
					LstBx_Usb_Keys_List->Items->Add(System::String(DriveStr[0]) + String(": RAW"));
					ListBoxCopy->Items->Add(System::String(DriveStr[0]) + String(": RAW"));
					LstBx_Usb_Keys_Drive_Letter->Items->Add(DriveStr[0]);
					LstBx_Usb_Key_Locked->Items->Add(String(" "));
					LstBx_Usb_Key_UnLocked->Items->Add(L"UNLOCKED !");
					LstBx_Usb_Key_Status->Items->Add(L"...");
				}
			}
		}
	}
}

void __fastcall TUSB_RAW::Btn_Erase_Usb_KeyClick(TObject *Sender)
{

	int selectedIndex = LstBx_Usb_Keys_List->ItemIndex;
	if (selectedIndex != -1)
	{
		Btn_Quitter->Visible = false;
		Interrompre = false;
		EdtBx_Erasing_Selected_Usb_Key->Text = LstBx_Usb_Keys_List->Items->Strings[selectedIndex];

		UnicodeString selectedDrive = LstBx_Usb_Keys_List->Items->Strings[selectedIndex];
		wchar_t driveLetter = selectedDrive[1];  // Extraire la lettre de lecteur

		wchar_t DriveStr[7] = { L'\\', L'\\', L'.', L'\\', driveLetter, L':', L'\0' };

		// Ouvrir le périphérique pour écriture brute
		HANDLE hDrive = CreateFile(DriveStr, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hDrive == INVALID_HANDLE_VALUE)
		{
			ShowMessage(L"L'ouverture de la lettre de lecteur pour écriture a échouée.");
			return;
		}

		// Positionner le curseur de fichier au début
		if (SetFilePointer(hDrive, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			ShowMessage(L"Echec de définition du pointeur.");
			CloseHandle(hDrive);
			return;
		}

		// Obtenir la géométrie du disque pour déterminer sa taille
		DISK_GEOMETRY diskGeometry;
		DWORD bytesReturned;
		if (!DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &diskGeometry, sizeof(diskGeometry), &bytesReturned, NULL))
		{
			ShowMessage(L"Echec pour connaître la taille du disque.");
			CloseHandle(hDrive);
			return;
		}

		ULONGLONG totalSize = diskGeometry.Cylinders.QuadPart * diskGeometry.TracksPerCylinder * diskGeometry.SectorsPerTrack * diskGeometry.BytesPerSector;

		// Initialiser la barre de progression
		const DWORD bufferSize = 4096; // Déclaration de bufferSize
		Progress_Formating_Usb_Key->Max = totalSize / bufferSize;
		Progress_Formating_Usb_Key->Position = 0;
		Lbl_Progress_Level->Caption = L"0%"; // Initialiser le texte du label

		// Écrire 11111111 en binaire sur toute la clé USB
		BYTE buffer[bufferSize];
		memset(buffer, 0xFF, bufferSize);  // Remplir le buffer avec 11111111 en binaire

		DWORD bytesWritten;
		ULONGLONG totalBytesWritten = 0;
		while (WriteFile(hDrive, buffer, bufferSize, &bytesWritten, NULL) && bytesWritten == bufferSize)
		{
			totalBytesWritten += bytesWritten;
			Progress_Formating_Usb_Key->Position = totalBytesWritten / bufferSize;

			// Calculer le pourcentage d'avancement
			int percentComplete = (totalBytesWritten * 100) / totalSize;
			Lbl_Progress_Level->Caption = IntToStr(percentComplete) + L"%";

			Application->ProcessMessages();  // Rafraîchir l'interface utilisateur

			// Vérifier si l'interruption est demandée
			if ((Interrompre && Btn_Quitter->Visible && !Quitter))
			{
				Quitter = true;
				ShowMessage(L"Programme interrompu par l'utilisateur.");
				break; // Sortir de la boucle
			}

			// Ajouter un délai pour voir la progression plus clairement
			Sleep(50);  // Délai en millisecondes (100 ms)
		}

		CloseHandle(hDrive);
		ShowMessage(L"USB drive effacée avec succès !");
	}
	else
	{
		ShowMessage(L"Aucune Lettre de lecteur sélectionnée.");
	}
}


void __fastcall TUSB_RAW::LstBx_Usb_Keys_Drive_LetterClick(TObject *Sender)
{
int selectedIndex = LstBx_Usb_Keys_Drive_Letter->ItemIndex;

LstBx_Usb_Keys_List->ItemIndex = selectedIndex;
ListBoxCopy->ItemIndex = selectedIndex;
LstBx_Usb_Key_UnLocked->ItemIndex = selectedIndex;
LstBx_Usb_Key_Locked->ItemIndex = selectedIndex;
LstBx_Usb_Key_Status->ItemIndex = selectedIndex;
EdtBx_Selected_Usb_Key->Text = LstBx_Usb_Keys_List->Items->Strings[selectedIndex];
Btn_Kill_MBR_GPT->Enabled = true;
Btn_Usb_Keys_Show->Enabled = false;
Btn_Annuler_Retour->Visible = true;
}
//-
void __fastcall TUSB_RAW::LstBx_Usb_Keys_ListClick(TObject *Sender)
{
int selectedIndex = LstBx_Usb_Keys_List->ItemIndex;

ListBoxCopy->ItemIndex = selectedIndex;
LstBx_Usb_Key_UnLocked->ItemIndex = selectedIndex;
LstBx_Usb_Key_Locked->ItemIndex = selectedIndex;
LstBx_Usb_Keys_Drive_Letter->ItemIndex = selectedIndex;
LstBx_Usb_Key_Status->ItemIndex = selectedIndex;
EdtBx_Selected_Usb_Key->Text = LstBx_Usb_Keys_List->Items->Strings[selectedIndex];
Btn_Kill_MBR_GPT->Enabled = true;
Btn_Usb_Keys_Show->Enabled = false;
Btn_Annuler_Retour->Visible = true;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void __fastcall TUSB_RAW::LstBx_Usb_Key_UnLockedClick(TObject *Sender)
{
int selectedIndex = LstBx_Usb_Key_UnLocked->ItemIndex;

LstBx_Usb_Keys_Drive_Letter->ItemIndex = selectedIndex;
LstBx_Usb_Keys_List->ItemIndex = selectedIndex;
ListBoxCopy->ItemIndex = selectedIndex;
LstBx_Usb_Key_Locked->ItemIndex = selectedIndex;
LstBx_Usb_Key_Status->ItemIndex = selectedIndex;
EdtBx_Selected_Usb_Key->Text = LstBx_Usb_Keys_List->Items->Strings[selectedIndex];
Btn_Kill_MBR_GPT->Enabled = true;
Btn_Usb_Keys_Show->Enabled = false;
Btn_Annuler_Retour->Visible = true;
}
//---------------------------------------------------------------------------

void __fastcall TUSB_RAW::ListBoxCopyClick(TObject *Sender)
{
int selectedIndex = ListBoxCopy->ItemIndex;

LstBx_Usb_Keys_Drive_Letter->ItemIndex = selectedIndex;
LstBx_Usb_Keys_List->ItemIndex = selectedIndex;
LstBx_Usb_Key_Locked->ItemIndex = selectedIndex;
LstBx_Usb_Key_UnLocked->ItemIndex = selectedIndex;
LstBx_Usb_Key_Status->ItemIndex = selectedIndex;
EdtBx_Selected_Usb_Key->Text = LstBx_Usb_Keys_List->Items->Strings[selectedIndex];
Btn_Kill_MBR_GPT->Enabled = true;
Btn_Usb_Keys_Show->Enabled = false;
Btn_Annuler_Retour->Visible = true;
}
//---------------------------------------------------------------------------

void __fastcall TUSB_RAW::LstBx_Usb_Key_LockedClick(TObject *Sender)
{
int selectedIndex = LstBx_Usb_Key_Locked->ItemIndex;

LstBx_Usb_Keys_Drive_Letter->ItemIndex = selectedIndex;
LstBx_Usb_Keys_List->ItemIndex = selectedIndex;
ListBoxCopy->ItemIndex = selectedIndex;
LstBx_Usb_Key_UnLocked->ItemIndex = selectedIndex;
LstBx_Usb_Key_Status->ItemIndex = selectedIndex;
EdtBx_Selected_Usb_Key->Text = LstBx_Usb_Keys_List->Items->Strings[selectedIndex];
Btn_Kill_MBR_GPT->Enabled = true;
Btn_Usb_Keys_Show->Enabled = false;
Btn_Annuler_Retour->Visible = true;
}


void __fastcall TUSB_RAW::LstBx_Usb_Key_StatusClick(TObject *Sender)
{
int selectedIndex = LstBx_Usb_Key_Status->ItemIndex;

LstBx_Usb_Keys_Drive_Letter->ItemIndex = selectedIndex;
LstBx_Usb_Keys_List->ItemIndex = selectedIndex;
ListBoxCopy->ItemIndex = selectedIndex;
LstBx_Usb_Key_UnLocked->ItemIndex = selectedIndex;
LstBx_Usb_Key_Locked->ItemIndex = selectedIndex;
EdtBx_Selected_Usb_Key->Text = LstBx_Usb_Keys_List->Items->Strings[selectedIndex];
Btn_Kill_MBR_GPT->Enabled = true;
Btn_Usb_Keys_Show->Enabled = false;
Btn_Annuler_Retour->Visible = true;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void __fastcall TUSB_RAW::Btn_Kill_MBR_GPTClick(TObject *Sender)
{
	int selectedIndex = USB_RAW->ListBoxCopy->ItemIndex;
	int CountElements = USB_RAW->LstBx_Usb_Key_Status->GetCount();
	int selectedElement = USB_RAW->LstBx_Usb_Key_Status->ItemIndex;
	
	// Obtenez la lettre du lecteur USB
	wchar_t driveLetter[] = L"E"; // Remplacez par la lettre du lecteur USB que vous souhaitez verrouiller
 if (UnlockUsbDrive(driveLetter, this))
 {


	   if (selectedIndex != -1)
	   {
		// Succès
		Edit13->Text = L"USB drive Unlocked successfully for Erasing in Progress !";
		EdtBx_Erasing_Selected_Usb_Key->Text = LstBx_Usb_Keys_List->Items->Strings[selectedIndex];
		EdtBx_Effacement_En_Cours->Text = "Effacement en cours !";
		 for (int i = 0; i <= CountElements - 1; ++i)
		 {
			if (i == selectedElement)
			{
					 LstBx_Usb_Key_Status->Items->Strings[i] = "...ERASING...";
			}
			else
			{
					 LstBx_Usb_Key_Status->Items->Strings[i] = "...WAIT...";
			}
		 }



		Btn_Quitter->Visible = false;
		Interrompre = false;
		Btn_Kill_MBR_GPT->Enabled = False;
		UnicodeString selectedDrive = ListBoxCopy->Items->Strings[selectedIndex];

		wchar_t driveLetter = selectedDrive[1];  // Extraire la lettre de lecteur

		wchar_t DriveStr[7] = { L'\\', L'\\', L'.', L'\\', driveLetter, L':', L'\0' };

		// Ouvrir le périphérique pour écriture brute
		HANDLE hDrive = CreateFile(DriveStr, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hDrive == INVALID_HANDLE_VALUE)
		{
			ShowMessage(L"L'ouverture de la lettre de lecteur pour écriture a échouée.");
			return;
		}

		// Obtenir la géométrie du disque pour déterminer sa taille
		DISK_GEOMETRY diskGeometry;
		DWORD bytesReturned;
		if (!DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &diskGeometry, sizeof(diskGeometry), &bytesReturned, NULL))
		{
			ShowMessage(L"Ne parvient pas à acquérir un Lecteur.");
			CloseHandle(hDrive);
			return;
		}

		ULONGLONG totalSize = diskGeometry.Cylinders.QuadPart * diskGeometry.TracksPerCylinder * diskGeometry.SectorsPerTrack * diskGeometry.BytesPerSector;

		// Initialiser la barre de progression
		const DWORD bufferSize = 4096; // Déclaration de bufferSize
		Progress_Formating_Usb_Key->Max = totalSize / bufferSize;
		Progress_Formating_Usb_Key->Position = 0;
		Lbl_Progress_Level->Caption = L"0%"; // Initialiser le texte du label

		// Écrire des zéros sur toute la clé USB pour la rendre "RAW"
		BYTE buffer[bufferSize];
		memset(buffer, 0xFF, bufferSize);  // Remplir le buffer avec des FF

		DWORD bytesWritten;
		ULONGLONG totalBytesWritten = 0;
		while (WriteFile(hDrive, buffer, bufferSize, &bytesWritten, NULL) && bytesWritten == bufferSize)
		{
			totalBytesWritten += bytesWritten;
			Progress_Formating_Usb_Key->Position = totalBytesWritten / bufferSize;

			// Calculer le pourcentage d'avancement
			int percentComplete = (totalBytesWritten * 100) / totalSize;
			Lbl_Progress_Level->Caption = IntToStr(percentComplete) + L"%";

			Application->ProcessMessages();  // Rafraîchir l'interface utilisateur

			// Vérifier si l'interruption est demandée
			if (Interrompre && Btn_Quitter->Visible && Quitter)
			{
				Quitter = true;
				ShowMessage(L"programme interrompu par l'utilisateur...");
				break; // Sortir de la boucle
			}

			// Ajouter un délai pour voir la progression plus clairement
			Sleep(50);  // Délai en millisecondes (100 ms)
		}

		CloseHandle(hDrive);

			if ((Interrompre && Btn_Quitter->Visible && !Quitter))
			{

				ShowMessage(L"Clef USB mise en mode RAW!");
			}
	}
	else {
		ShowMessage(L"Aucune Lettre de lecteur sélectionnée.");
	}

   Btn_Erase_Usb_Key->Enabled = True;
 }
else {			// Échec
			Edit13->Text = L"Failed to Unlock the USB drive.";
	 }



 }

//---------------------------------------------------------------------------

void __fastcall TUSB_RAW::Btn_Annuler_RetourClick(TObject *Sender)
{
Btn_Quitter->Visible = true;
Interrompre = true; // Réinitialiser l'interruption
Btn_Kill_MBR_GPT->Enabled = false;
Btn_Erase_Usb_Key->Enabled = false;
Btn_Usb_Keys_Show->Enabled = true;
Btn_Usb_Keys_Show->Visible = true;
Btn_Annuler_Retour->Visible = false;
LstBx_Usb_Keys_List->Items->Clear();
LstBx_Usb_Keys_List->ItemIndex = -1;
LstBx_Usb_Keys_Drive_Letter->Items->Clear();
LstBx_Usb_Keys_Drive_Letter->ItemIndex = -1;
LstBx_Usb_Key_UnLocked->Items->Clear();
LstBx_Usb_Key_UnLocked->ItemIndex = -1;ListBoxCopy->Items->Clear();
ListBoxCopy->ItemIndex = -1;
LstBx_Usb_Key_Locked->Items->Clear();
LstBx_Usb_Key_Locked->ItemIndex = -1;
LstBx_Usb_Key_Status->Items->Clear();
LstBx_Usb_Key_Status->ItemIndex = -1;
EdtBx_Erasing_Selected_Usb_Key->Text = "Pas de Clef en effacement ...";
EdtBx_Effacement_En_Cours->Text = "Rien en cours ...";

}
//---------------------------------------------------------------------------

void __fastcall TUSB_RAW::Btn_QuitterClick(TObject *Sender)
{
				Quitter = true;
				Interrompre = true;
				USB_RAW->Close();


}

//---------------------------------------------------------------------------

void __fastcall TUSB_RAW::Btn_AutoRunAutoPLay_STOPClick(TObject *Sender)
{
	if (DisableAutoRun(this)) {
		Edit4->Text = L"\r\n AutoRun a été désactivé sur tous les lecteurs.\r\n";
	} else {
		Edit4->Text = L"\r\n Échec de la désactivation d'AutoRun. Une erreur Code 5 signifie qu'il fallait exécuter en admin\r\n";
	}
}
//---------------------------------------------------------------------------


void __fastcall TUSB_RAW::Btn_AutoRunAutoPLay_STARTClick(TObject *Sender)
{
	if (EnableAutoRun(this)) {
		Edit8->Text = L"\r\n AutoRun a été activé sur tous les lecteurs.\r\n";
	} else {
		Edit8->Text = L"\r\n Échec de l'activation d'AutoRun. Une erreur Code 5 signifie qu'il fallait exécuter en admin\r\n";
	}
}
//---------------------------------------------------------------------------



//--------------------------------------------------------------------------

void __fastcall TUSB_RAW::Btn_Lock_USB_KEYClick(TObject *Sender)
{
	  // Obtenez la lettre du lecteur USB
	wchar_t driveLetter[] = L"E"; // Remplacez par la lettre du lecteur USB que vous souhaitez verrouiller
	int CountElements = USB_RAW->LstBx_Usb_Key_Status->GetCount();
	int selectedIndex = LstBx_Usb_Keys_List->ItemIndex;
	if (CountElements == 0)
	{
		ShowMessage(L"Eh !!! Tu n'as pas chargé la LISTE des Clef USB connectées à ton Pc, mon ami, c'est ici : ---> Montrer les Clefs USB Reliées au PC : ... <--- !!! ;)");
	}
	else
	 {
		 // Appelez la fonction LockUsbDrive avec les paramètres appropriés
		 if (selectedIndex == -1)
		  {
		ShowMessage(L"Eh !!! Tu n'as pas sélectionné dans la LISTE des Clef USB connectées à ton Pc, la clef de ton choix, mon ami ! c'est dans les zone de listes !!! ;)");		 }
		   else
		   {

		   if (LstBx_Usb_Key_Locked->Items->Strings[selectedIndex] != "LOCKED !")
		   {

			if (LockUsbDrive(driveLetter, this))
			{
					// Succès
						Edit13->Text = L"USB drive locked successfully!";
			}
			else
			{
					// Échec
						Edit13->Text = L"Failed to lock the USB drive.";
			}

			}
			else
			{
			   ShowMessage(L"Eh Tu as déjà cette clef à l'état LOCKED !!! Mais : Tu as demandé à la LOCKED de nouveau,ça ne sert à rien, mon ami ;)");
			}
		 }


	 }





}

//---------------------------------------------------------------------------

void __fastcall TUSB_RAW::Btn_UnLock_USB_KEYClick(TObject *Sender)
{
	  // Obtenez la lettre du lecteur USB
	wchar_t driveLetter[] = L"E"; // Remplacez par la lettre du lecteur USB que vous souhaitez verrouiller
	int CountElements = USB_RAW->LstBx_Usb_Key_Status->GetCount();
	int selectedIndex = LstBx_Usb_Keys_List->ItemIndex;
	if (CountElements == 0)
	{
		ShowMessage(L"Eh !!! Tu n'as pas chargé la LISTE des Clef USB connectées à ton Pc, mon ami, c'est ici : ---> Montrer les Clefs USB Reliées au PC : ... <--- !!! ;)");
	}
	else
	 {
		 // Appelez la fonction LockUsbDrive avec les paramètres appropriés
		 if (selectedIndex == -1)
		  {
		ShowMessage(L"Eh !!! Tu n'as pas sélectionné dans la LISTE des Clef USB connectées à ton Pc, la clef de ton choix, mon ami ! c'est dans les zone de listes !!! ;)");
		 }
		   else
		   {

		   if (LstBx_Usb_Key_UnLocked->Items->Strings[selectedIndex] != "UNLOCKED !")
		   {

			if (UnlockUsbDrive(driveLetter, this))
			{
					// Succès
						Edit13->Text = L"USB drive locked successfully!";
			}
			else
			{
					// Échec
						Edit13->Text = L"Failed to lock the USB drive.";
			}

			}
			else
			{
			   ShowMessage(L"Eh Tu as déjà cette clef à l'état UNLOCKED !!! Mais : Tu as demandé à la UNLOCKED de nouveau,ça ne sert à rien, mon ami ;)");
			}
		 }


	 }





}




