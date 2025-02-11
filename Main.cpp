#include <vcl.h>
#include <System.IOUtils.hpp>
#include <windows.h> // Include Windows API header
#include <winioctl.h> // Inclure winioctl.h pour DISK_GEOMETRY
#include <setupapi.h>
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

bool Interrompre = false; // Variable globale pour signaler l'interruption
bool Quitter = false; // Variable globale pour signaler l'interruption

bool CycleUSBPort(const GUID* guid)
{
	HDEVINFO deviceInfoSet = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (deviceInfoSet == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Echec de récupération des infos de réglages du device." << std::endl;
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
			std::cerr << "Echec dans l'allocation de mémoire." << std::endl;
			continue;
		}

		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, requiredSize, &requiredSize, &deviceInfoData))
		{
			std::cerr << "Echec de récupération des infos de réglages du device." << std::endl;
			free(deviceInterfaceDetailData);
			continue;
		}

		HANDLE deviceHandle = CreateFile(deviceInterfaceDetailData->DevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (deviceHandle == INVALID_HANDLE_VALUE)
		{
			std::cerr << "Echec d'ouverture du device." << std::endl;
			free(deviceInterfaceDetailData);
			continue;
		}

		free(deviceInterfaceDetailData);

		DWORD bytesReturned = 0;
		if (!DeviceIoControl(deviceHandle, IOCTL_USB_CYCLE_PORT, NULL, 0, NULL, 0, &bytesReturned, NULL))
		{
			std::cerr << "Balayage des USB port des devices a échoué." << std::endl;
			CloseHandle(deviceHandle);
			continue;
		}

		CloseHandle(deviceHandle);
		SetupDiDestroyDeviceInfoList(deviceInfoSet);
		return true;
	}

	SetupDiDestroyDeviceInfoList(deviceInfoSet);
	std::cerr << "Ne trouve pas l'USB device." << std::endl;
	return false;
}

bool RestartUSBDevice()
{
	HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (deviceInfoSet == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Echec de récupération des infos de réglages du device." << std::endl;
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
			std::cerr << "Echec dans l'allocation de mémoire." << std::endl;
			continue;
		}

		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, requiredSize, &requiredSize, &deviceInfoData))
		{
			std::cerr << "N'a pas pu récupérer le détail de l'interface du device." << std::endl;
			free(deviceInterfaceDetailData);
			continue;
		}

		HANDLE deviceHandle = CreateFile(deviceInterfaceDetailData->DevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (deviceHandle == INVALID_HANDLE_VALUE)
		{
			std::cerr << "Echec de tentative d'ouverture du device." << std::endl;
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
				std::cerr << "Echec de la tentative d'éjection du device." << attempt + 1 << ")." << std::endl;
				continue;
			}

			Sleep(1000);

			cr = CM_Reenumerate_DevNode(deviceInfoData.DevInst, 0);
			if (cr != CR_SUCCESS)
			{
				std::cerr << "Echec de tentative de l'énumération des devices." << attempt + 1 << ")." << std::endl;
				continue;
			}

			// Successfully restarted the device
			std::cout << "Device redémarré avec succès avec cette tentative " << attempt + 1 << "." << std::endl;
			CloseHandle(deviceHandle);
			SetupDiDestroyDeviceInfoList(deviceInfoSet);
			return true;
		}

		CloseHandle(deviceHandle);
	}

	SetupDiDestroyDeviceInfoList(deviceInfoSet);
	std::cerr << "Le programme ne trouve pas l'USB device." << std::endl;
	return false;
}

void __fastcall TUSB_RAW::Btn_Usb_Keys_ShowClick(TObject *Sender)
{
	LstBx_Usb_Keys_List->Items->Clear();
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
				}
				else
				{
					LstBx_Usb_Keys_List->Items->Add(System::String(DriveStr[0]) + String(": RAW"));
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
		EdtBx_Selected_Usb_Key->Text = LstBx_Usb_Keys_List->Items->Strings[selectedIndex];

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
			Sleep(100);  // Délai en millisecondes (100 ms)
		}

		CloseHandle(hDrive);
		ShowMessage(L"USB drive effacée avec succès !");
	}
	else
	{
		ShowMessage(L"Aucune Lettre de lecteur sélectionnée.");
	}
}

void __fastcall TUSB_RAW::LstBx_Usb_Keys_ListClick(TObject *Sender)
{
int selectedIndex = LstBx_Usb_Keys_List->ItemIndex;
EdtBx_Selected_Usb_Key->Text = LstBx_Usb_Keys_List->Items->Strings[selectedIndex];
Btn_Kill_MBR_GPT->Enabled = true;
Btn_Usb_Keys_Show->Enabled = false;
Btn_Annuler_Retour->Visible = true;
}
//---------------------------------------------------------------------------

void __fastcall TUSB_RAW::Btn_Kill_MBR_GPTClick(TObject *Sender)
{
	int selectedIndex = LstBx_Usb_Keys_List->ItemIndex;
	if (selectedIndex != -1)
	{
		Btn_Quitter->Visible = false;
		Interrompre = false;
		Btn_Kill_MBR_GPT->Enabled = False;
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
			Sleep(100);  // Délai en millisecondes (100 ms)
		}

		CloseHandle(hDrive);

			if ((Interrompre && Btn_Quitter->Visible && !Quitter))
			{

				ShowMessage(L"Clef USB mise en mode RAW!");
			}
	}
	else
	{
		ShowMessage(L"Aucune Lettre de lecteur sélectionnée.");
	}

Btn_Erase_Usb_Key->Enabled = True;
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

}
//---------------------------------------------------------------------------

void __fastcall TUSB_RAW::Btn_QuitterClick(TObject *Sender)
{
				Quitter = true;
				Interrompre = true;
				USB_RAW->Close();


}

//--------------------------------------------------------------------------
