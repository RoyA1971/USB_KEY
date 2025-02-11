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
	TLabel *Lbl_Select_List; // Ajout de la barre de progression
	void __fastcall Btn_Usb_Keys_ShowClick(TObject *Sender);
	void __fastcall Btn_Kill_MBR_GPTClick(TObject *Sender);
	void __fastcall Btn_Erase_Usb_KeyClick(TObject *Sender);
	void __fastcall Btn_Annuler_RetourClick(TObject *Sender);
	void __fastcall Btn_QuitterClick(TObject *Sender);

	void __fastcall LstBx_Usb_Keys_ListClick(TObject *Sender);

public:        // Méthodes accessibles publiquement
	__fastcall TUSB_RAW(TComponent* Owner);
};

extern PACKAGE TUSB_RAW *USB_RAW;

#endif


