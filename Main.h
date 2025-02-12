#ifndef MainH
#define MainH

#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp> // Inclure les contrôles VCL

class TUSB_RAW : public TForm
{
__published:    // Composants gérés par Delphi/C++Builder
	TButton *Btn_Usb_Keys_Show;
	TButton *Btn_Kill_MBR_GPT;
	TButton *Btn_Erase_Usb_Key;
	TButton *Btn_Annuler_Retour;
	TButton *Btn_Quitter;

	TListBox *LstBx_Usb_Keys_List;

	TEdit *EdtBx_Selected_Usb_Key;

	TProgressBar *Progress_Formating_Usb_Key;

	TLabel *Lbl_Selected_Usb_Key;
	TLabel *Lbl_Progress_Level;
	TLabel *Lbl_Select_List;
	TEdit *Edit3;
	TEdit *Edit2;
	TEdit *Edit1;
	TEdit *Edit4;
	TEdit *Edit5;
	TEdit *Edit6;
	TEdit *Edit7;
	TEdit *Edit8;
	TButton *Btn_AutoRunAutoPLay_STOP;
	TButton *Btn_AutoRunAutoPLay_START;
	TButton *Btn_Lock_USB_KEY;
	TEdit *Edit9;
	TEdit *Edit10;
	TEdit *Edit11;
	TEdit *Edit12;
	TEdit *Edit13;
	TListBox *LstBx_Usb_Keys_Drive_Letter;
	TButton *Btn_UnLock_USB_KEY;
	TListBox *ListBoxCopy;
	TListBox *LstBx_Usb_Key_UnLocked;
	TListBox *LstBx_Usb_Key_Locked;
	TEdit *EdtBx_Erasing_Selected_Usb_Key;
	TLabel *Lbl_Erasing_Key;
	TEdit *EdtBx_Effacement_En_Cours;
	TLabel *Label1;
	TListBox *LstBx_Usb_Key_Status;
	TLabel *Label2;
	TLabel *Lbl_Version; // Ajout de la barre de progression
	void __fastcall Btn_Usb_Keys_ShowClick(TObject *Sender);
	void __fastcall Btn_Kill_MBR_GPTClick(TObject *Sender);
	void __fastcall Btn_Erase_Usb_KeyClick(TObject *Sender);
	void __fastcall Btn_Annuler_RetourClick(TObject *Sender);
	void __fastcall Btn_QuitterClick(TObject *Sender);

	void __fastcall LstBx_Usb_Keys_ListClick(TObject *Sender);
	void __fastcall Btn_AutoRunAutoPLay_STOPClick(TObject *Sender);
	void __fastcall Btn_AutoRunAutoPLay_STARTClick(TObject *Sender);
	void __fastcall Btn_Lock_USB_KEYClick(TObject *Sender);
	void __fastcall LstBx_Usb_Keys_Drive_LetterClick(TObject *Sender);
	void __fastcall Btn_UnLock_USB_KEYClick(TObject *Sender);
	void __fastcall LstBx_Usb_Key_UnLockedClick(TObject *Sender);
	void __fastcall ListBoxCopyClick(TObject *Sender);
	void __fastcall LstBx_Usb_Key_LockedClick(TObject *Sender);
	void __fastcall LstBx_Usb_Key_StatusClick(TObject *Sender);

public:        // Méthodes accessibles publiquement
	__fastcall TUSB_RAW(TComponent* Owner);
};

extern PACKAGE TUSB_RAW *USB_RAW;

#endif
