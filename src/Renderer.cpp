#include "Renderer.hpp"

#include "Action.hpp"
#include "Direct.hpp"
#include "Game.hpp"
#include "Math.hpp"
#include "Menu.hpp"
#include "Sound.hpp"
#include "Routing.hpp"
#include "World.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>

#include <SFML/Window.hpp>

#define MB_LEFT sf::Mouse::Left;

namespace Renderer {
double pi = 3.1415926535; // pi, was sonst

void Fade(short RP, short GP, short BP)
{
    for (short blackloop = 0; blackloop < 256; blackloop++) {
        DDGammaRamp.red[blackloop] = DDGammaOld.red[blackloop] * RP / 100;
        DDGammaRamp.green[blackloop] = DDGammaOld.green[blackloop] * GP / 100;
        DDGammaRamp.blue[blackloop] = DDGammaOld.blue[blackloop] * BP / 100;
    }

    lpDDGammaControl->SetGammaRamp(0, &DDGammaRamp);
}

void LimitScroll()
{
    if (Camera.x < ScapeGrenze.left) {
        Camera.x = static_cast<short>(ScapeGrenze.left);
    }

    if (Camera.x + rcSpielflaeche.right > ScapeGrenze.right) {
        Camera.x = static_cast<short>(ScapeGrenze.right) - static_cast<short>(rcSpielflaeche.right);
    }

    if (Camera.y < ScapeGrenze.top) {
        Camera.y = static_cast<short>(ScapeGrenze.top);
    }

    if (Camera.y + rcSpielflaeche.bottom > ScapeGrenze.bottom) {
        Camera.y = static_cast<short>(ScapeGrenze.bottom) - static_cast<short>(rcSpielflaeche.bottom);
    }
}

ZWEID GetKachel(short PosX, short PosY)
{
    ZWEID Erg;

    for (short y = 0; y < MAXYKACH; y++)
        for (short x = 0; x < MAXXKACH; x++) {
            // Die in Betracht kommenden Kacheln rausfinden
            if ((PosX > Scape[x][y].xScreen) && (PosX < Scape[x][y].xScreen + KXPIXEL) &&
                    (PosY > Scape[x][y].yScreen) && (PosY < Scape[x][y].yScreen + KYPIXEL)) {
                if ((Math::InDreieck(PosX, PosY,
                                     Scape[x][y].xScreen + EckKoor[Scape[x][y].Typ][0].x,
                                     Scape[x][y].yScreen + EckKoor[Scape[x][y].Typ][0].y,
                                     Scape[x][y].xScreen + EckKoor[Scape[x][y].Typ][1].x,
                                     Scape[x][y].yScreen + EckKoor[Scape[x][y].Typ][1].y,
                                     Scape[x][y].xScreen + EckKoor[Scape[x][y].Typ][3].x,
                                     Scape[x][y].yScreen + EckKoor[Scape[x][y].Typ][3].y)) ||
                        (Math::InDreieck(PosX, PosY,
                                         Scape[x][y].xScreen + EckKoor[Scape[x][y].Typ][2].x,
                                         Scape[x][y].yScreen + EckKoor[Scape[x][y].Typ][2].y,
                                         Scape[x][y].xScreen + EckKoor[Scape[x][y].Typ][1].x,
                                         Scape[x][y].yScreen + EckKoor[Scape[x][y].Typ][1].y,
                                         Scape[x][y].xScreen + EckKoor[Scape[x][y].Typ][3].x,
                                         Scape[x][y].yScreen + EckKoor[Scape[x][y].Typ][3].y))) {
                    Erg.x = x;
                    Erg.y = y;
                    return Erg;
                }
            }
        }

    Erg.x = -1;
    Erg.y = -1;
    return Erg;
}

inline DWORD RGB2DWORD(BYTE r, BYTE g, BYTE b)
{
    DWORD Erg;

    if (ddpf.dwRBitMask == 63488) {
        Erg = static_cast<DWORD>((r & 0xF8) >> 3);
        Erg = Erg << 6;
        Erg = Erg | static_cast<DWORD>((g & 0xFC) >> 2);
        Erg = Erg << 5;
        Erg = Erg | static_cast<DWORD>((b & 0xF8) >> 3);
    } else if (ddpf.dwRBitMask == 31744) {
        Erg = static_cast<DWORD>((r & 0xF8) >> 3);
        Erg = Erg << 5;
        Erg = Erg | static_cast<DWORD>((g & 0xF8) >> 3);
        Erg = Erg << 5;
        Erg = Erg | static_cast<DWORD>((b & 0xF8) >> 3);
    } else if (ddpf.dwRBitMask == 16711680) {
        Erg = static_cast<DWORD>(r & 0xFF);
        Erg = Erg << 8;
        Erg = Erg | static_cast<DWORD>(g & 0xFF);
        Erg = Erg << 8;
        Erg = Erg | static_cast<DWORD>(b & 0xFF);
    } else {
        Erg = 0;
//            MessageBeep(MB_OK);
    }

    return Erg;
}

inline void DWORD2RGB(DWORD color)
{
    if (ddpf.dwRBitMask == 63488) {
        rgbStruct.r = static_cast<byte>((color & 0xF800) >> 8);
        rgbStruct.g = static_cast<byte>((color & 0x07E0) >> 3);
        rgbStruct.b = static_cast<byte>((color & 0x001F) << 3);
    } else if (ddpf.dwRBitMask == 31744) {
        rgbStruct.r = static_cast<byte>((color & 0x7C00) >> 7);
        rgbStruct.g = static_cast<byte>((color & 0x03E0) >> 2);
        rgbStruct.b = static_cast<byte>((color & 0x001F) << 3);
    } else if (ddpf.dwRBitMask == 16711680) {
        rgbStruct.r = static_cast<byte>((color & 0xFF0000) >> 10);
        rgbStruct.g = static_cast<byte>((color & 0x00FF00) >> 8);
        rgbStruct.b = static_cast<byte>((color & 0x0000FF));
    }
}

void Blitten(LPDIRECTDRAWSURFACE4 lpDDSVon, LPDIRECTDRAWSURFACE4 lpDDSNach, bool Transp)
{
    HRESULT hr;
    short z = 0;

    while (true) {
        z++;
        hr = lpDDSNach->GetBltStatus(/*DDGBS_ISBLTDONE |*/ DDGBS_CANBLT);

        if (hr == DD_OK) {
            break;
        }

        Sleep(1);

        if (z == 1000) {
            MessageBeep(MB_OK);
            break;
        }
    }

    while (true) {
        z++;

        if (Transp) {
            hr = lpDDSNach->Blt(&rcRectdes, lpDDSVon, &rcRectsrc, DDBLT_KEYSRC | DDBLT_WAIT, nullptr);
        } else {
            hr = lpDDSNach->Blt(&rcRectdes, lpDDSVon, &rcRectsrc, DDBLT_WAIT, nullptr);
        }

        if (hr != DDERR_WASSTILLDRAWING) {
            break;
        }

        if (z == 1000) {
            MessageBeep(MB_OK);
            break;
        }
    }
}

void PutPixel(short x, short y, DWORD color, LPDDSURFACEDESC2 ddsd)
{
    WORD *pixels = static_cast<WORD *>(ddsd->lpSurface);
    // DWORD pitch = ddsd->dwWidth+2;
    DWORD pitch = ddsd->lPitch >> 1;
    pixels[y * pitch + x * 2] = static_cast<WORD>(color);
}

void GetPixel(short x, short y, LPDDSURFACEDESC2 ddsd)
{
    WORD *pixels = static_cast<WORD *>(ddsd->lpSurface);
    // DWORD pitch = ddsd->dwWidth;
    DWORD pitch = ddsd->lPitch >> 1;
    DWORD color = pixels[y * pitch + x * 2];

    DWORD2RGB(color);
}

void ZeichneBilder(short x, short y, short i, RECT Ziel, bool Reverse, short Frucht)
{
    short Phase;

    if (Frucht == -1) {
        Phase = Bmp[i].Phase;
    } else {
        Phase = Frucht;
    }

    rcRectsrc = Bmp[i].rcSrc;

    if (!Reverse) {
        rcRectsrc.top += Phase * (Bmp[i].Hoehe);
    } else {
        rcRectsrc.top = Bmp[i].rcSrc.top + (Bmp[i].Anzahl - 1) * Bmp[i].Hoehe - Phase * Bmp[i].Hoehe;
    }

    rcRectsrc.bottom = rcRectsrc.top + (Bmp[i].Hoehe);
    rcRectdes.left = x;
    rcRectdes.top = y;
    rcRectdes.right = x + (Bmp[i].Breite);
    rcRectdes.bottom = y + (Bmp[i].Hoehe);
    Math::CalcRect(Ziel);
    Blitten(Bmp[i].Surface, lpDDSBack, true);
}

void ZeichneObjekte()
{
    for (short y = 0; y < MAXYKACH; y++)
        for (short x = 0; x < MAXXKACH; x++) {
            bool Guyzeichnen = false;

            if ((Guy.Pos.x == x) && (Guy.Pos.y == y)) {
                Guyzeichnen = true;
            }

            // Die nichtsichbaren Kacheln (oder nicht betroffenen) ausfiltern

            if (!((Scape[x][y].xScreen > Camera.x + rcSpielflaeche.left - KXPIXEL) &&
                    (Scape[x][y].xScreen < Camera.x + rcSpielflaeche.right + KXPIXEL) &&
                    (Scape[x][y].yScreen > Camera.y + rcSpielflaeche.top - KYPIXEL) &&
                    (Scape[x][y].yScreen < Camera.y + rcSpielflaeche.bottom + KYPIXEL) &&
                    (Scape[x][y].Entdeckt) &&
                    ((Scape[x][y].Markiert) || (Scape[x][y].Objekt != -1) || (Guyzeichnen)))) {
                continue;
            }

            if (Scape[x][y].Markiert) { // Die Rahmen um die markierten Kacheln malen
                rcRectsrc.left = KXPIXEL * Scape[x][y].Typ;
                rcRectsrc.right = KXPIXEL * Scape[x][y].Typ + KXPIXEL;
                rcRectsrc.top = 2 * KYPIXEL;
                rcRectsrc.bottom = 3 * KYPIXEL;
                rcRectdes.left = Scape[x][y].xScreen - Camera.x;
                rcRectdes.right = rcRectdes.left + KXPIXEL;
                rcRectdes.top = Scape[x][y].yScreen - Camera.y;
                rcRectdes.bottom = rcRectdes.top + KYPIXEL;
                Math::CalcRect(rcSpielflaeche);
                Blitten(lpDDSMisc, lpDDSBack, true);
            }

            // Landschaftsanimationen malen (und Feld)
            if ((Scape[x][y].Objekt != -1) && (LAnimation) &&
                    ((Scape[x][y].Objekt <= SCHLEUSE6))
                    || (Scape[x][y].Objekt == FELD) // Der Guy ist immer vor diesen Objekten
                    || (Scape[x][y].Objekt == ROHR)
                    || (Scape[x][y].Objekt == SOS)) {
                // Sound abspielen
                if (Scape[x][y].Objekt != -1 &&
                        ((Guy.Pos.x - 1 <= x) && (x <= Guy.Pos.x + 1)) &&
                        ((Guy.Pos.y - 1 <= y) && (y <= Guy.Pos.y + 1))) {
                    if ((x == Guy.Pos.x) && (y == Guy.Pos.y)) {
                        Sound::PlaySound(Bmp[Scape[x][y].Objekt].Sound, 100);
                    } else if (Bmp[Scape[x][y].Objekt].Sound != Bmp[Scape[Guy.Pos.x][Guy.Pos.y].Objekt].Sound) {
                        Sound::PlaySound(Bmp[Scape[x][y].Objekt].Sound, 90);
                    }
                }

                ZeichneBilder(Scape[x][y].xScreen + Scape[x][y].ObPos.x - Camera.x,
                              Scape[x][y].yScreen + Scape[x][y].ObPos.y - Camera.y,
                              Scape[x][y].Objekt, rcSpielflaeche, Scape[x][y].Reverse,
                              static_cast<short>(Scape[x][y].Phase));
            } else {
                if (((Scape[x][y].Objekt >= BAUM1) && (Scape[x][y].Objekt <= BAUM4DOWN)) ||
                        (Scape[x][y].Objekt == BAUMGROSS) || (Scape[x][y].Objekt == FEUER) ||
                        (Scape[x][y].Objekt == WRACK) || (Scape[x][y].Objekt == WRACK2) ||
                        (Scape[x][y].Objekt >= ZELT)) { // Bäume und Früchte (und alle anderen Objekte) malen
                    // Sound abspielen
                    if (Scape[x][y].Objekt != -1 &&
                            ((Guy.Pos.x - 1 <= x) && (x <= Guy.Pos.x + 1)) &&
                            ((Guy.Pos.y - 1 <= y) && (y <= Guy.Pos.y + 1))) {
                        if ((x == Guy.Pos.x) && (y == Guy.Pos.y)) {
                            Sound::PlaySound(Bmp[Scape[x][y].Objekt].Sound, 100);
                        } else if (Bmp[Scape[x][y].Objekt].Sound != Bmp[Scape[Guy.Pos.x][Guy.Pos.y].Objekt].Sound) {
                            Sound::PlaySound(Bmp[Scape[x][y].Objekt].Sound, 90);
                        }
                    }

                    if (Guyzeichnen) {
                        if ((Guy.PosScreen.y) < (Scape[x][y].yScreen + Scape[x][y].ObPos.y
                                                 + Bmp[Scape[x][y].Objekt].Hoehe)) {
                            ZeichneGuy();
                            Guyzeichnen = false;
                        }
                    }

                    ZeichneBilder(Scape[x][y].xScreen + Scape[x][y].ObPos.x - Camera.x,
                                  Scape[x][y].yScreen + Scape[x][y].ObPos.y - Camera.y,
                                  Scape[x][y].Objekt, rcSpielflaeche, false,
                                  static_cast<short>(Scape[x][y].Phase));
                }
            }

            if (Guyzeichnen) {
                ZeichneGuy();
            }
        }
}

void ZeichneGuy()
{
    if (BootsFahrt) {
        if (Guy.Zustand == GUYSCHIFF) {
            ZeichneBilder(Guy.PosScreen.x - 30 - Camera.x,
                          Guy.PosScreen.y - 28 - Camera.y,
                          Guy.Zustand, rcSpielflaeche, false, -1);
        } else {
            ZeichneBilder(Guy.PosScreen.x - (Bmp[Guy.Zustand].Breite) / 2 - Camera.x,
                          Guy.PosScreen.y - (Bmp[Guy.Zustand].Hoehe) / 2 - Camera.y,
                          Guy.Zustand, rcSpielflaeche, false, -1);
        }
    } else
        ZeichneBilder(Guy.PosScreen.x - (Bmp[Guy.Zustand].Breite) / 2 - Camera.x,
                      Guy.PosScreen.y - (Bmp[Guy.Zustand].Hoehe) - Camera.y,
                      Guy.Zustand, rcSpielflaeche, false, -1);

    // Sound abspielen
    if (Guy.Aktiv) {
        Sound::PlaySound(Bmp[Guy.Zustand].Sound, 100);
    }
}

void ZeichnePapier()
{
    rcRectsrc.left = 0;
    rcRectsrc.top = 0;
    rcRectsrc.right = 464;
    rcRectsrc.bottom = 77;
    rcRectdes.left = TextBereich[TXTPAPIER].rcText.left - 60;
    rcRectdes.top = TextBereich[TXTPAPIER].rcText.top - 30;
    rcRectdes.right = rcRectdes.left + 464;
    rcRectdes.bottom = rcRectdes.top + 77;
    Blitten(lpDDSPapier, lpDDSBack, true);
    rcRectdes.left = rcRectdes.left + 34;
    rcRectdes.top = rcRectdes.top + 77;
    rcRectdes.right = rcRectdes.right;
    rcRectdes.bottom = TextBereich[TXTPAPIER].rcText.top + PapierText;
    ddbltfx.dwFillColor = RGB2DWORD(236, 215, 179);
    lpDDSBack->Blt(&rcRectdes, nullptr, nullptr, DDBLT_COLORFILL, &ddbltfx);
    rcRectsrc.left = 0;
    rcRectsrc.top = 77;
    rcRectsrc.right = 464;
    rcRectsrc.bottom = 154;
    rcRectdes.left = TextBereich[TXTPAPIER].rcText.left - 60;
    rcRectdes.top = rcRectdes.bottom - 47;
    rcRectdes.right = rcRectdes.left + 464;
    rcRectdes.bottom = rcRectdes.top + 77;
    Blitten(lpDDSPapier, lpDDSBack, true);
}

void ZeichnePanel()
{
    // Karte
    rcRectsrc.left = 0;
    rcRectsrc.top = 0;
    rcRectsrc.right = 2 * MAXXKACH;
    rcRectsrc.bottom = 2 * MAXYKACH;
    rcRectdes.left = rcKarte.left;
    rcRectdes.top = rcKarte.top;
    rcRectdes.right = rcKarte.right;
    rcRectdes.bottom = rcKarte.bottom;
    Blitten(lpDDSKarte, lpDDSBack, false);

    // Spielfigur
    rcRectdes.left = rcKarte.left + 2 * Guy.Pos.x;
    rcRectdes.top = rcKarte.top + 2 * Guy.Pos.y;
    rcRectdes.right = rcRectdes.left + 2;
    rcRectdes.bottom = rcRectdes.top + 2;
    ddbltfx.dwFillColor = RGB2DWORD(255, 0, 0);
    lpDDSBack->Blt(&rcRectdes, nullptr, nullptr, DDBLT_COLORFILL, &ddbltfx);

    // Position einmalen
    rcRectsrc.left = 205;
    rcRectsrc.top = 0;
    rcRectsrc.right = 205 + 65;
    rcRectsrc.bottom = 0 + 65;
    rcRectdes.left = rcKarte.left + (Camera.x + 2 * Camera.y) / (KXPIXEL / 2) - MAXXKACH - 2;
    rcRectdes.top = rcKarte.top + (2 * Camera.y - Camera.x) / (KXPIXEL / 2) + MAXYKACH - 21 - 2;
    rcRectdes.right = rcRectdes.left + 65;
    rcRectdes.bottom = rcRectdes.top + 65;
    Math::CalcRect(rcKarte);
    Blitten(lpDDSPanel, lpDDSBack, true);

    // Panel malen
    rcRectsrc.left = 0;
    rcRectsrc.top = 0;
    rcRectsrc.right = 205;
    rcRectsrc.bottom = 720;
    rcRectdes.left = rcPanel.left;
    rcRectdes.top = rcPanel.top;
    rcRectdes.right = rcPanel.right;
    rcRectdes.bottom = rcPanel.bottom;
    Blitten(lpDDSPanel, lpDDSBack, true);

    // Gitternetzknopf
    if (Gitter) {
        Bmp[BUTTGITTER].Phase = 1;
    } else {
        Bmp[BUTTGITTER].Phase = 0;
    }

    ZeichneBilder(static_cast<short>(Bmp[BUTTGITTER].rcDes.left),
                  static_cast<short>(Bmp[BUTTGITTER].rcDes.top),
                  BUTTGITTER, rcPanel, false, -1);

    // SOUNDknopf
    if ((Soundzustand == 0) || (Soundzustand == -1)) {
        Bmp[BUTTSOUND].Phase = 1;
    } else {
        Bmp[BUTTSOUND].Phase = 0;
    }

    ZeichneBilder(static_cast<short>(Bmp[BUTTSOUND].rcDes.left),
                  static_cast<short>(Bmp[BUTTSOUND].rcDes.top),
                  BUTTSOUND, rcPanel, false, -1);

    // ANIMATIONknopf
    if (!LAnimation) {
        Bmp[BUTTANIMATION].Phase = 1;
    } else {
        Bmp[BUTTANIMATION].Phase = 0;
    }

    ZeichneBilder(static_cast<short>(Bmp[BUTTANIMATION].rcDes.left),
                  static_cast<short>(Bmp[BUTTANIMATION].rcDes.top),
                  BUTTANIMATION, rcPanel, false, -1);

    // BEENDENknopf
    ZeichneBilder(static_cast<short>(Bmp[BUTTBEENDEN].rcDes.left),
                  static_cast<short>(Bmp[BUTTBEENDEN].rcDes.top),
                  BUTTBEENDEN, rcPanel, false, -1);

    // NEUknopf
    ZeichneBilder(static_cast<short>(Bmp[BUTTNEU].rcDes.left),
                  static_cast<short>(Bmp[BUTTNEU].rcDes.top),
                  BUTTNEU, rcPanel, false, -1);

    // TAGNEUknopf
    ZeichneBilder(static_cast<short>(Bmp[BUTTTAGNEU].rcDes.left),
                  static_cast<short>(Bmp[BUTTTAGNEU].rcDes.top),
                  BUTTTAGNEU, rcPanel, false, -1);

    // Aktionsknopf
    if (HauptMenue == Menu::ACTION) {
        Bmp[BUTTAKTION].Phase = Bmp[BUTTAKTION].Anzahl;
    } else if (Bmp[BUTTAKTION].Phase == Bmp[BUTTAKTION].Anzahl) {
        Bmp[BUTTAKTION].Phase = 0;
    }

    ZeichneBilder(static_cast<short>(Bmp[BUTTAKTION].rcDes.left),
                  static_cast<short>(Bmp[BUTTAKTION].rcDes.top),
                  BUTTAKTION, rcPanel, false, -1);

    // BauKnopf
    if (HauptMenue == Menu::BUILD) {
        Bmp[BUTTBAUEN].Phase = Bmp[BUTTBAUEN].Anzahl;
    } else if (Bmp[BUTTBAUEN].Phase == Bmp[BUTTBAUEN].Anzahl) {
        Bmp[BUTTBAUEN].Phase = 0;
    }

    ZeichneBilder(static_cast<short>(Bmp[BUTTBAUEN].rcDes.left),
                  static_cast<short>(Bmp[BUTTBAUEN].rcDes.top),
                  BUTTBAUEN, rcPanel, false, -1);

    // Inventarknopf
    if (HauptMenue == Menu::INVENTORY) {
        Bmp[BUTTINVENTAR].Phase = Bmp[BUTTINVENTAR].Anzahl;
    } else if (Bmp[BUTTINVENTAR].Phase == Bmp[BUTTINVENTAR].Anzahl) {
        Bmp[BUTTINVENTAR].Phase = 0;
    }

    ZeichneBilder(static_cast<short>(Bmp[BUTTINVENTAR].rcDes.left),
                  static_cast<short>(Bmp[BUTTINVENTAR].rcDes.top),
                  BUTTINVENTAR, rcPanel, false, -1);

    // WEITERknopf
    if (Bmp[BUTTWEITER].Phase != -1)
        ZeichneBilder(static_cast<short>(Bmp[BUTTWEITER].rcDes.left),
                      static_cast<short>(Bmp[BUTTWEITER].rcDes.top),
                      BUTTWEITER, rcPanel, false, -1);

    // STOPknopf
    if (Bmp[BUTTSTOP].Phase != -1)
        ZeichneBilder(static_cast<short>(Bmp[BUTTSTOP].rcDes.left),
                      static_cast<short>(Bmp[BUTTSTOP].rcDes.top),
                      BUTTSTOP, rcPanel, false, -1);

    // ABLEGENknopf
    if (Bmp[BUTTABLEGEN].Phase != -1)
        ZeichneBilder(static_cast<short>(Bmp[BUTTABLEGEN].rcDes.left),
                      static_cast<short>(Bmp[BUTTABLEGEN].rcDes.top),
                      BUTTABLEGEN, rcPanel, false, -1);

    // Welches Menü zeichnen?
    switch (HauptMenue) {
    case Menu::ACTION:
        for (short i = BUTTSUCHEN; i <= BUTTSCHLEUDER; i++) {
            if (Bmp[i].Phase == -1) {
                ZeichneBilder(static_cast<short>(Bmp[i].rcDes.left),
                              static_cast<short>(Bmp[i].rcDes.top),
                              BUTTFRAGEZ, rcPanel, false, -1);
                continue;
            }

            ZeichneBilder(static_cast<short>(Bmp[i].rcDes.left),
                          static_cast<short>(Bmp[i].rcDes.top),
                          i, rcPanel, false, -1);
        }

        break;

    case Menu::BUILD:
        for (short i = BUTTZELT; i <= BUTTDESTROY; i++) {
            if (Bmp[i].Phase == -1) {
                ZeichneBilder(static_cast<short>(Bmp[i].rcDes.left),
                              static_cast<short>(Bmp[i].rcDes.top),
                              BUTTFRAGEZ, rcPanel, false, -1);
                continue;
            }

            ZeichneBilder(static_cast<short>(Bmp[i].rcDes.left),
                          static_cast<short>(Bmp[i].rcDes.top),
                          i, rcPanel, false, -1);
        }

        break;

    case Menu::INVENTORY:
        ZeichneBilder(static_cast<short>(Bmp[INVPAPIER].rcDes.left),
                      static_cast<short>(Bmp[INVPAPIER].rcDes.top),
                      INVPAPIER, rcPanel, false, -1);

        for (short i = ROHAST; i <= ROHSCHLEUDER; i++) {
            if (Guy.Inventar[i] <= 0) {
                continue;
            }

            ZeichneBilder(static_cast<short>(Bmp[i].rcDes.left),
                          static_cast<short>(Bmp[i].rcDes.top),
                          i, rcPanel, false, -1);
            Bmp[ROEMISCH1].rcDes.top = Bmp[i].rcDes.top;
            Bmp[ROEMISCH2].rcDes.top = Bmp[i].rcDes.top;

            for (short j = 1; j <= Guy.Inventar[i]; j++) {
                if (j < 5) {
                    ZeichneBilder(static_cast<short>(Bmp[i].rcDes.left) + 20 + j * 4,
                                  static_cast<short>(Bmp[ROEMISCH1].rcDes.top),
                                  ROEMISCH1, rcPanel, false, -1);
                } else if (j == 5)
                    ZeichneBilder(static_cast<short>(Bmp[i].rcDes.left) + 23,
                                  static_cast<short>(Bmp[ROEMISCH2].rcDes.top),
                                  ROEMISCH2, rcPanel, false, -1);
                else if ((j > 5) && (j < 10)) {
                    ZeichneBilder(static_cast<short>(Bmp[i].rcDes.left) + 20 + j * 4,
                                  static_cast<short>(Bmp[ROEMISCH1].rcDes.top),
                                  ROEMISCH1, rcPanel, false, -1);
                } else if (j == 10)
                    ZeichneBilder(static_cast<short>(Bmp[i].rcDes.left) + 43,
                                  static_cast<short>(Bmp[ROEMISCH2].rcDes.top),
                                  ROEMISCH2, rcPanel, false, -1);
            }
        }

        break;
    }

    // Säule1
    short i = Bmp[SAEULE1].Hoehe - static_cast<short>(Guy.Resource[WASSER]) * Bmp[SAEULE1].Hoehe / 100;
    rcRectsrc = Bmp[SAEULE1].rcSrc;
    rcRectsrc.top += i;
    rcRectdes = Bmp[SAEULE1].rcDes;
    rcRectdes.top += i;
    Blitten(Bmp[SAEULE1].Surface, lpDDSBack, true);

    // Säule2
    i = Bmp[SAEULE2].Hoehe - static_cast<short>(Guy.Resource[NAHRUNG]) * Bmp[SAEULE2].Hoehe / 100;
    rcRectsrc = Bmp[SAEULE2].rcSrc;
    rcRectsrc.top += i;
    rcRectdes = Bmp[SAEULE2].rcDes;
    rcRectdes.top += i;
    Blitten(Bmp[SAEULE2].Surface, lpDDSBack, true);

    // Säule3
    i = Bmp[SAEULE3].Hoehe - static_cast<short>(Guy.Resource[GESUNDHEIT]) * Bmp[SAEULE3].Hoehe / 100;
    rcRectsrc = Bmp[SAEULE3].rcSrc;
    rcRectsrc.top += i;
    rcRectdes = Bmp[SAEULE3].rcDes;
    rcRectdes.top += i;
    Blitten(Bmp[SAEULE3].Surface, lpDDSBack, true);

    // Sonnenanzeige
    short diffx = (static_cast<short>(Bmp[SONNE].rcDes.right) - static_cast<short>(Bmp[SONNE].rcDes.left) - Bmp[SONNE].Breite) / 2;
    short diffy = static_cast<short>(Bmp[SONNE].rcDes.bottom) - static_cast<short>(Bmp[SONNE].rcDes.top) - Bmp[SONNE].Hoehe / 2;
    short TagesZeit = (Stunden * 10 + Minuten * 10 / 60);

    ZeichneBilder(static_cast<short>(Bmp[SONNE].rcDes.left + diffx * cos(pi - pi * TagesZeit / 120) + diffx),
                  static_cast<short>(Bmp[SONNE].rcDes.top + (-diffy * sin(pi - pi * TagesZeit / 120) + diffy)),
                  SONNE, Bmp[SONNE].rcDes, false, -1);

    // Rettungsring
    short Ringtmp;

    if (Chance < 100) {
        Ringtmp = static_cast<short>(100 * sin(pi / 200 * Chance));
    } else {
        Ringtmp = 100;
    }

    if (Ringtmp > 100) {
        Ringtmp = 100;
    }

    ZeichneBilder(static_cast<short>(Bmp[RING].rcDes.left),
                  static_cast<short>(Bmp[RING].rcDes.top + Ringtmp),
                  RING, rcPanel, false, -1);

    // Die ChanceZahl ausgeben
    Textloeschen(TXTCHANCE);
    TextBereich[TXTCHANCE].Aktiv = true;
    TextBereich[TXTCHANCE].rcText.top = Bmp[RING].rcDes.top + Ringtmp + Bmp[RING].Hoehe;
    TextBereich[TXTCHANCE].rcText.bottom = TextBereich[TXTCHANCE].rcText.top + S2YPIXEL;
    std::sprintf(StdString, "%.1f", Chance);
    DrawString(StdString, static_cast<short>(TextBereich[TXTCHANCE].rcText.left),
               static_cast<short>(TextBereich[TXTCHANCE].rcText.top), 2);

    // TextFeld malen
    rcRectsrc.left = 0;
    rcRectsrc.top = 0;
    rcRectsrc.right = 605;
    rcRectsrc.bottom = 20;
    rcRectdes = {0, MAXY - 20, MAXX - 195, MAXY};
    Blitten(lpDDSTextFeld, lpDDSBack, false);
}

void DrawString(char *string, short x, short y, short Art)
{
    short Breite = 0;
    short Hoehe = 0;

    if (Art == 1) {
        Breite = S1XPIXEL;
        Hoehe = S1YPIXEL;
    }

    if (Art == 2) {
        Breite = S2XPIXEL;
        Hoehe = S2YPIXEL;
    }

    // Länge der Schrift ermitteln
    std::size_t length = strlen(string);

    // Alle Zeichen durchgehen
    for (std::size_t index = 0; index < length; index++) {
        // Korrekte indexNummer ermitteln
        short cindex = string[index] - ' ';

        if ((string[index] >= ' ') && (string[index] <= '/')) {
            rcRectsrc.left = cindex * Breite;
            rcRectsrc.top = 0;
        }

        if ((string[index] >= '0') && (string[index] <= '?')) {
            rcRectsrc.left = (cindex - 16) * Breite;
            rcRectsrc.top = Hoehe;
        }

        if ((string[index] >= '@') && (string[index] <= 'O')) {
            rcRectsrc.left = (cindex - 16 * 2) * Breite;
            rcRectsrc.top = 2 * Hoehe;
        }

        if ((string[index] >= 'P') && (string[index] <= '_')) {
            rcRectsrc.left = (cindex - 16 * 3) * Breite;
            rcRectsrc.top = 3 * Hoehe;
        }

        if ((string[index] > '_') && (string[index] <= 'o')) {
            rcRectsrc.left = (cindex - 16 * 4) * Breite;
            rcRectsrc.top = 4 * Hoehe;
        }

        if ((string[index] >= 'p') && (string[index] <= '~')) {
            rcRectsrc.left = (cindex - 16 * 5) * Breite;
            rcRectsrc.top = 5 * Hoehe;
        }

        rcRectsrc.right = rcRectsrc.left + Breite;
        rcRectsrc.bottom = rcRectsrc.top + Hoehe;
        rcRectdes.left = x;
        rcRectdes.top = y;
        rcRectdes.right = x + Breite;
        rcRectdes.bottom = y + Hoehe;

        // Zeichen zeichnen
        if (Art == 1) {
            Blitten(lpDDSSchrift1, lpDDSSchrift, true);
            // x Position weiterschieben
            x += S1ABSTAND;
        }

        if (Art == 2) {
            Blitten(lpDDSSchrift2, lpDDSSchrift, true);
            // x Position weiterschieben
            x += S2ABSTAND;
        }
    }
}

short DrawText(int TEXT, short Bereich, short Art)
{
    short BBreite = 0;
    short BHoehe = 0;
    char Text[1024];
    int blank = ' ';
    int slash = '/';
    int strend = 0x0;
    char StdString2[10]; // Zur Variablenausgabe
    short Anzahl; // Zur Variablenausgabe

    Textloeschen(Bereich);
    TextBereich[Bereich].Aktiv = true;

    if (Art == 1) {
        BBreite = S1ABSTAND;
        BHoehe = S1YPIXEL;
    }

    if (Art == 2) {
        BBreite = S2ABSTAND;
        BHoehe = S2YPIXEL;
    }

    LoadString(g_hInst, TEXT, Text, 1024);
    short Posx = static_cast<short>(TextBereich[Bereich].rcText.left);
    short Posy = static_cast<short>(TextBereich[Bereich].rcText.top);
    char *Posnext = Text;

    while (true) {
        strcpy(StdString, "");
        short Pos = Posnext - Text;
        Posnext = strchr(Text + Pos + 1, blank);
        char *Posnext2 = strchr(Text + Pos + 1, slash);

        if ((Posnext != nullptr) && (Posnext2 != nullptr) && (Posnext2 <= Posnext)) {
            char scratch = *(Posnext2 + 1);

            switch (scratch) {
            case 'a':
                Anzahl = std::sprintf(StdString2, " %d", Tag);
                DrawString(StdString2, Posx, Posy, Art);
                Posx += BBreite * (Anzahl);
                break;

            case 'b':
                Anzahl = std::sprintf(StdString2, " %d", static_cast<short>(Guy.Resource[GESUNDHEIT]));
                DrawString(StdString2, Posx, Posy, Art);
                Posx += BBreite * (Anzahl);
                break;

            case 'c':
                Anzahl = std::sprintf(StdString2, " %.2f", Chance);
                DrawString(StdString2, Posx, Posy, Art);
                Posx += BBreite * (Anzahl);
                break;

            case 'd':
                Frage = 0;
                rcRectsrc = Bmp[JA].rcSrc;
                rcRectdes.left = static_cast<short>(TextBereich[Bereich].rcText.left) + 50;
                rcRectdes.top = Posy + 50;
                rcRectdes.right = rcRectdes.left + Bmp[JA].Breite;
                rcRectdes.bottom = rcRectdes.top + Bmp[JA].Hoehe;
                Bmp[JA].rcDes = rcRectdes;
                Blitten(Bmp[JA].Surface, lpDDSSchrift, false);

                rcRectsrc = Bmp[NEIN].rcSrc;
                rcRectdes.left = static_cast<short>(TextBereich[Bereich].rcText.left) + 220;
                rcRectdes.top = Posy + 50;
                rcRectdes.right = rcRectdes.left + Bmp[NEIN].Breite;
                rcRectdes.bottom = rcRectdes.top + Bmp[NEIN].Hoehe;
                Bmp[NEIN].rcDes = rcRectdes;
                Blitten(Bmp[NEIN].Surface, lpDDSSchrift, false);
                Posy += 115;
                break;

            case 'z':
                Posx = static_cast<short>(TextBereich[Bereich].rcText.left) - BBreite;
                Posy += BHoehe + 3;
                break;
            }

            Pos = Pos + 3;
            Posnext = Posnext2 + 2;
        }

        if (Posnext == nullptr) {
            Posnext = strchr(Text + Pos + 1, strend);
        }

        strncpy(StdString, Text + Pos, (Posnext - Text) - Pos);

        if (Posx + BBreite * ((Posnext - Text) - Pos) > TextBereich[Bereich].rcText.right) {
            Posx = static_cast<short>(TextBereich[Bereich].rcText.left) - BBreite;
            Posy += BHoehe + 3;
        }

        StdString[(Posnext - Text) - Pos] = static_cast<char>(0);
        DrawString(StdString, Posx, Posy, Art);

        if (Posnext[0] == static_cast<char>(0)) {
            break;
        }

        Posx += BBreite * ((Posnext - Text) - Pos);
    }

    short Erg = static_cast<short>(Posy + BHoehe - TextBereich[Bereich].rcText.top);

    if (Erg < 100) {
        Erg = 100;
    }

    return Erg;
}

void Textloeschen(short Bereich)
{
    TextBereich[Bereich].Aktiv = false;
    ddbltfx.dwFillColor = RGB2DWORD(255, 0, 255);
    lpDDSSchrift->Blt(&TextBereich[Bereich].rcText, nullptr, nullptr, DDBLT_COLORFILL, &ddbltfx);
}

void DrawSchatzkarte()
{
    Textloeschen(TXTPAPIER);
    TextBereich[TXTPAPIER].Aktiv = true;
    PapierText = SKARTEY;

    rcRectsrc.left = 0;
    rcRectsrc.right = SKARTEX;
    rcRectsrc.top = 0;
    rcRectsrc.bottom = SKARTEY;
    rcRectdes.left = TextBereich[TXTPAPIER].rcText.left;
    rcRectdes.top = TextBereich[TXTPAPIER].rcText.top;
    rcRectdes.right = rcRectdes.left + SKARTEX;
    rcRectdes.bottom = rcRectdes.top + SKARTEY;

    Blitten(lpDDSSchatzkarte, lpDDSSchrift, false);
}

void Zeige()
{
    char Stringsave1[128], Stringsave2[128]; // Für die Zeitausgabe

    rcRectsrc.left = Camera.x + rcSpielflaeche.left;
    rcRectsrc.top = Camera.y + rcSpielflaeche.top;
    rcRectsrc.right = Camera.x + rcSpielflaeche.right;
    rcRectsrc.bottom = Camera.y + rcSpielflaeche.bottom;
    rcRectdes.left = rcSpielflaeche.left;
    rcRectdes.top = rcSpielflaeche.top;
    rcRectdes.right = rcSpielflaeche.right;
    rcRectdes.bottom = rcSpielflaeche.bottom;

    Blitten(lpDDSScape, lpDDSBack, false); // Landschaft zeichnen

    ZeichneObjekte();

    ZeichnePanel();

    // Die TagesZeit ausgeben
    Textloeschen(TXTTAGESZEIT);
    TextBereich[TXTTAGESZEIT].Aktiv = true;
    std::sprintf(Stringsave1, "%d", Stunden + 6);
    std::sprintf(Stringsave2, "%d", Minuten);
    strcpy(StdString, "");

    if (Stunden + 6 < 10) {
        strcat(StdString, "0");
    }

    strcat(StdString, Stringsave1);
    strcat(StdString, ":");

    if (Minuten < 10) {
        strcat(StdString, "0");
    }

    strcat(StdString, Stringsave2);
    DrawString(StdString, static_cast<short>(TextBereich[TXTTAGESZEIT].rcText.left),
               static_cast<short>(TextBereich[TXTTAGESZEIT].rcText.top), 2);

    if (PapierText != -1) {
        ZeichnePapier();
    }

    // Die Textsurface blitten
    for (short i = 0; i < TEXTANZ; i++) {
        if (!TextBereich[i].Aktiv) {
            continue;    // Die nicht aktiven Felder auslassen
        }

        rcRectsrc = TextBereich[i].rcText;
        rcRectdes = TextBereich[i].rcText;
        Blitten(lpDDSSchrift, lpDDSBack, true);
    }

    // Alles schwarz übermalen und nur das Papier mit Text anzeigen
    if (Nacht) {
        rcRectdes.left = 0;
        rcRectdes.top = 0;
        rcRectdes.right = MAXX;
        rcRectdes.bottom = MAXY;
        ddbltfx.dwFillColor = RGB2DWORD(0, 0, 0);
        lpDDSBack->Blt(&rcRectdes, nullptr, nullptr, DDBLT_COLORFILL, &ddbltfx);

        if (PapierText != -1) {
            ZeichnePapier();
            rcRectsrc = TextBereich[TXTPAPIER].rcText;
            rcRectdes = TextBereich[TXTPAPIER].rcText;
            Blitten(lpDDSSchrift, lpDDSBack, true);
        }

        Fade(100, 100, 100);
    }

    // Cursor
    if (CursorTyp == CUPFEIL)
        ZeichneBilder(MousePosition.x, MousePosition.y,
                      CursorTyp, rcGesamt, false, -1);
    else
        ZeichneBilder(MousePosition.x - Bmp[CursorTyp].Breite / 2,
                      MousePosition.y - Bmp[CursorTyp].Hoehe / 2,
                      CursorTyp, rcGesamt, false, -1);

    // Flippen
    while (true) {
        HRESULT ddrval = lpDDSPrimary->Flip(nullptr, 0);

        if (ddrval == DD_OK) {
            break;
        }

        if (ddrval == DDERR_SURFACELOST) {
            ddrval = lpDDSPrimary->Restore();

            if (ddrval != DD_OK) {
                break;
            }
        }

        if (ddrval != DDERR_WASSTILLDRAWING) {
            break;
        }
    }


    if (Nacht) {
        Fade(100, 100, 100);    // Das muß hier stehen, damit man die Textnachricht in der Nacht lesen kann
    }
}

void ZeigeIntro()
{
    rcRectdes.left = 0;
    rcRectdes.top = 0;
    rcRectdes.right = MAXX;
    rcRectdes.bottom = MAXY;
    ddbltfx.dwFillColor = RGB2DWORD(0, 0, 0);
    short z = 0;

    while (true) {
        z++;
        HRESULT ddrval = lpDDSBack->Blt(&rcRectdes, nullptr, nullptr, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

        if (ddrval != DDERR_WASSTILLDRAWING) {
            break;
        }

        if (z == 1000) {
            MessageBeep(MB_OK);
            break;
        }
    }

    rcRectsrc.left = Camera.x + rcSpielflaeche.left;
    rcRectsrc.top = Camera.y + rcSpielflaeche.top;
    rcRectsrc.right = Camera.x + rcSpielflaeche.right;
    rcRectsrc.bottom = Camera.y + rcSpielflaeche.bottom;
    rcRectdes.left = rcSpielflaeche.left;
    rcRectdes.top = rcSpielflaeche.top;
    rcRectdes.right = rcSpielflaeche.right;
    rcRectdes.bottom = rcSpielflaeche.bottom;

    Blitten(lpDDSScape, lpDDSBack, false); // Landschaft zeichnen

    ZeichneObjekte();

    if (PapierText != -1) {
        ZeichnePapier();
    }

    // Die Textsurface blitten
    for (short i = 0; i < TEXTANZ; i++) {
        if (!TextBereich[i].Aktiv) {
            continue;    // Die nicht aktiven Felder auslassen
        }

        rcRectsrc = TextBereich[i].rcText;
        rcRectdes = TextBereich[i].rcText;
        Blitten(lpDDSSchrift, lpDDSBack, true);
    }

    // Flippen
    while (true) {
        HRESULT ddrval = lpDDSPrimary->Flip(nullptr, 0);

        if (ddrval == DD_OK) {
            break;
        }

        if (ddrval == DDERR_SURFACELOST) {
            ddrval = lpDDSPrimary->Restore();

            if (ddrval != DD_OK) {
                break;
            }
        }

        if (ddrval != DDERR_WASSTILLDRAWING) {
            break;
        }
    }
}

void ZeigeAbspann()
{
    PlaySound(Sound::OUTRO, 100);

    rcRectdes.left = 0;
    rcRectdes.top = 0;
    rcRectdes.right = MAXX;
    rcRectdes.bottom = MAXY;
    ddbltfx.dwFillColor = RGB2DWORD(0, 0, 0);
    short z = 0;

    while (true) {
        z++;
        HRESULT ddrval = lpDDSBack->Blt(&rcRectdes, nullptr, nullptr, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

        if (ddrval != DDERR_WASSTILLDRAWING) {
            break;
        }

        if (z == 1000) {
            MessageBeep(MB_OK);
            break;
        }
    }

    if (AbspannZustand == 0) {
        ZeichneBilder(static_cast<short>(MAXX) / 2 - Bmp[AbspannListe[AbspannNr][0].Bild].Breite / 2, 100,
                      AbspannListe[AbspannNr][0].Bild, rcGesamt, false, -1);

        for (z = 1; z < 10; z++) {
            if (AbspannListe[AbspannNr][z].Aktiv)
                AbspannBlt(AbspannListe[AbspannNr][z].Bild,
                           static_cast<short>(100 * sin(pi / MAXY * (Bmp[AbspannListe[AbspannNr][z].Bild].rcDes.top +
                                                        Bmp[AbspannListe[AbspannNr][z].Aktiv].Hoehe / 2))));
        }
    } else if (AbspannZustand == 1) {
        rcRectsrc = Bmp[AbspannNr].rcSrc;
        rcRectsrc.top += Bmp[AbspannNr].Phase * Bmp[AbspannNr].Hoehe;
        rcRectsrc.bottom = rcRectsrc.top + Bmp[AbspannNr].Hoehe;

        rcRectdes.left = 2;
        rcRectdes.top = 2;
        rcRectdes.right = Bmp[AbspannNr].Breite + 2;
        rcRectdes.bottom = Bmp[AbspannNr].Hoehe + 2;

        Blitten(Bmp[AbspannNr].Surface, lpDDSBack, true);

        rcRectsrc.left = 0;
        rcRectsrc.top = 0;
        rcRectsrc.right = Bmp[AbspannNr].Breite + 4;
        rcRectsrc.bottom = Bmp[AbspannNr].Hoehe + 4;

        rcRectdes.left = static_cast<short>(MAXX) / 2 - rcRectsrc.right * 10 / 2;
        rcRectdes.top = static_cast<short>(MAXY) / 2 - rcRectsrc.bottom * 10 / 2;
        rcRectdes.right = rcRectdes.left + rcRectsrc.right * 10;
        rcRectdes.bottom = rcRectdes.top + rcRectsrc.bottom * 10;

        Blitten(lpDDSBack, lpDDSBack, false);

        rcRectsrc.left = 100;
        rcRectsrc.top = 2;
        rcRectsrc.right = 100 + Bmp[AbspannNr].Breite + 2;
        rcRectsrc.bottom = Bmp[AbspannNr].Hoehe + 2;

        rcRectdes.left = 2;
        rcRectdes.top = 2;
        rcRectdes.right = Bmp[AbspannNr].Breite + 2;
        rcRectdes.bottom = Bmp[AbspannNr].Hoehe + 2;

        Blitten(lpDDSBack, lpDDSBack, false);
    }

    // Flippen
    while (true) {
        HRESULT ddrval = lpDDSPrimary->Flip(nullptr, 0);

        if (ddrval == DD_OK) {
            break;
        }

        if (ddrval == DDERR_SURFACELOST) {
            ddrval = lpDDSPrimary->Restore();

            if (ddrval != DD_OK) {
                break;
            }
        }

        if (ddrval != DDERR_WASSTILLDRAWING) {
            break;
        }
    }
}

void ZeigeLogo()
{
    rcRectdes.left = 0;
    rcRectdes.top = 0;
    rcRectdes.right = MAXX;
    rcRectdes.bottom = MAXY;
    ddbltfx.dwFillColor = RGB2DWORD(0, 0, 0);
    short z = 0;

    while (true) {
        z++;
        HRESULT ddrval = lpDDSBack->Blt(&rcRectdes, nullptr, nullptr, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

        if (ddrval != DDERR_WASSTILLDRAWING) {
            break;
        }

        if (z == 1000) {
            MessageBeep(MB_OK);
            break;
        }
    }

    rcRectsrc.left = 0;
    rcRectsrc.right = 500;
    rcRectsrc.top = 0;
    rcRectsrc.bottom = 500;
    rcRectdes.left = MAXX / 2 - 250;
    rcRectdes.right = MAXX / 2 + 250;
    rcRectdes.top = MAXY / 2 - 250;
    rcRectdes.bottom = MAXY / 2 + 250;


    Blitten(lpDDSLogo, lpDDSBack, false);

    PlaySound(Sound::LOGO, 100);

    // Flippen
    while (true) {
        HRESULT ddrval = lpDDSPrimary->Flip(nullptr, 0);

        if (ddrval == DD_OK) {
            break;
        }

        if (ddrval == DDERR_SURFACELOST) {
            ddrval = lpDDSPrimary->Restore();

            if (ddrval != DD_OK) {
                break;
            }
        }

        if (ddrval != DDERR_WASSTILLDRAWING) {
            break;
        }
    }
}

void AbspannBlt(short Bild, short Prozent)
{
    Bmp[Bild].Surface->Lock(nullptr, &ddsd, DDLOCK_WAIT, nullptr);
    lpDDSBack->Lock(nullptr, &ddsd2, DDLOCK_WAIT, nullptr);

    for (short x = 0; x < Bmp[Bild].Breite; x++)
        for (short y = 0; y < Bmp[Bild].Hoehe; y++) {
            if ((x + Bmp[Bild].rcDes.left >= MAXX) || (x + Bmp[Bild].rcDes.left <= 0) ||
                    (y + Bmp[Bild].rcDes.top >= MAXY) || (y + Bmp[Bild].rcDes.top <= 0)) {
                continue;
            }

            Renderer::GetPixel(static_cast<short>(x + Bmp[Bild].rcDes.left),
                               static_cast<short>(y + Bmp[Bild].rcDes.top), &ddsd2);
            RGBSTRUCT rgbalt = rgbStruct;
            Renderer::GetPixel(static_cast<short>(x + Bmp[Bild].rcSrc.left),
                               static_cast<short>(y + Bmp[Bild].rcSrc.top), &ddsd);

            if ((rgbStruct.r == 0) && (rgbStruct.g == 0) && (rgbStruct.b == 0)) {
                continue;
            }

            PutPixel(static_cast<short>(x + Bmp[Bild].rcDes.left),
                     static_cast<short>(y + Bmp[Bild].rcDes.top),
                     RGB2DWORD(rgbalt.r + (rgbStruct.r - rgbalt.r) * Prozent / 100,
                               rgbalt.g + (rgbStruct.g - rgbalt.g) * Prozent / 100,
                               rgbalt.b + (rgbStruct.b - rgbalt.b) * Prozent / 100),
                     &ddsd2);
        }

    Bmp[Bild].Surface->Unlock(nullptr);
    lpDDSBack->Unlock(nullptr);
}
} // namespace Renderer
