#include "World.hpp"

#include "Action.hpp"
#include "Direct.hpp"
#include "Renderer.hpp"
#include "Sound.hpp"
#include "State.hpp"

#include <cstdio>
#include <cstring>

namespace World
{
    FLUSSLAUF Flusslauf[FLUSSANZAHL][MAXFLUSS];

    void MakeRohString(short x, short y, short Objekt)
    {
        char TmpString[1024];

        RohString[0] = char(0);
        bool keinRohstoff = true;
        if (Objekt == -1)
        {
            for (short i = 0; i < BILDANZ; i++)
            {
                if (Scape[x][y].Rohstoff[i] != 0) keinRohstoff = false;
            }
        }
        else
        {
            for (short i = 0; i < BILDANZ; i++)
            {
                if (Bmp[Objekt].Rohstoff[i] != 0) keinRohstoff = false;
            }
        }
        if (keinRohstoff) return;
        strcat(RohString, " ->");
        for (short i = 0; i < BILDANZ; i++)
        {
            if (Objekt == -1)
            {
                if (Scape[x][y].Rohstoff[i] == 0)
                    continue;
            }
            else
            {
                if (Bmp[Objekt].Rohstoff[i] == 0)
                    continue;
            }
            strcat(RohString, " ");
            switch (i)
            {
            case ROHAST: LoadString(g_hInst, AST, TmpString, 1024);
                break;
            case ROHSTEIN: LoadString(g_hInst, STEIN, TmpString, 1024);
                break;
            case ROHBLATT: LoadString(g_hInst, BLATT, TmpString, 1024);
                break;
            case ROHLIANE: LoadString(g_hInst, LIANE, TmpString, 1024);
                break;
            case ROHSTAMM: LoadString(g_hInst, STAMM, TmpString, 1024);
                break;
            }
            strcat(RohString, TmpString);
            strcat(RohString, "=");
            if (Objekt == -1) std::sprintf(TmpString, "%d", Scape[x][y].Rohstoff[i]);
            else std::sprintf(TmpString, "%d", Bmp[Objekt].Rohstoff[i]);
            strcat(RohString, TmpString);
        }
    }

    void AddTime(short h, short m)
    {
        Stunden += h;
        Minuten += m;
        if (Minuten >= 60)
        {
            Minuten -= 60;
            Stunden++;
        }
        for (short y = 0; y < MAXYKACH; y++)
            for (short x = 0; x < MAXXKACH; x++)
            {
                // Feuer nach einer bestimmten Zeit ausgehen lassen
                if (Scape[x][y].Objekt == FEUER)
                {
                    Scape[x][y].Timer += float((60 * h + m) * 0.0005);
                    if (Scape[x][y].Timer >= 1)
                    {
                        Scape[x][y].Objekt = -1;
                        Scape[x][y].Timer = 0;
                        Scape[x][y].ObPos.x = 0;
                        Scape[x][y].ObPos.y = 0;
                        Scape[x][y].Phase = -1;
                        Chance -= 2 + 2 * Scape[x][y].Hoehe;
                    }
                }
                if ((Scape[x][y].Phase == -1) ||
                    ((Scape[x][y].Objekt != FELD) &&
                        (Scape[x][y].Objekt != BUSCH)))
                    continue; // Wenn kein Fruchtobjekt weiter
                if (Scape[x][y].Phase >= Bmp[Scape[x][y].Objekt].Anzahl) continue;
                if (Scape[x][y].Objekt == FELD) Scape[x][y].Phase += float((60 * h + m) * 0.005);
                else if (Scape[x][y].Objekt == BUSCH) Scape[x][y].Phase += float((60 * h + m) * 0.0005); // pro Minute Reifungsprozess fortführen
                if (Scape[x][y].Phase > Bmp[Scape[x][y].Objekt].Anzahl - 1)
                    Scape[x][y].Phase = static_cast<float>(Bmp[Scape[x][y].Objekt].Anzahl) - 1;
            }
        AddResource(GESUNDHEIT, (60 * h + m) * (Guy.Resource[WASSER] - 50 + Guy.Resource[NAHRUNG] - 50) / 1000);

        if ((Spielzustand == State::GAME) && (!BootsFahrt))
        {
            for (short i = 0; i <= (60 * h + m); i++)
            {
                if (Chance == 0) break;
                if (rand() % static_cast<int>(1 / (Chance / 72000)) == 1)
                {
                    Guy.Aktiv = false;
                    Guy.AkNummer = 0;
                    Guy.Aktion = Action::RESCUED;
                    break;
                }
            }
        }
    }

    void AddResource(short Art, float Anzahl) // Fügt wassser usw hinzu
    {
        Guy.Resource[Art] += Anzahl;
        if (Guy.Resource[Art] > 100) Guy.Resource[Art] = 100;
        if (Guy.Resource[Art] < 0) Guy.Resource[Art] = 0;
        // Wann tod
        if ((Guy.Resource[GESUNDHEIT] <= 0) && (Guy.Aktion != Action::DEATH) &&
            (Guy.Aktion != Action::DAY_END) && (Spielzustand == State::GAME))
        {
            Guy.Aktiv = false;
            Guy.AkNummer = 0;
            Guy.Aktion = Action::DEATH;
        }
    }

    void Generate()
    {
        // Die Kartehintergrundfarbe
        rcRectdes.left = 0;
        rcRectdes.top = 0;
        rcRectdes.right = 2 * MAXXKACH;
        rcRectdes.bottom = 2 * MAXYKACH;
        ddbltfx.dwFillColor = Renderer::RGB2DWORD(247, 222, 191);
//        lpDDSKarte->Blt(&rcRectdes, nullptr, nullptr, DDBLT_COLORFILL, &ddbltfx);

        // Die Landschaftshintergrundfarbe
        rcRectdes.left = 0;
        rcRectdes.top = 0;
        rcRectdes.right = MAXSCAPEX;
        rcRectdes.bottom = MAXSCAPEY;
        ddbltfx.dwFillColor = Renderer::RGB2DWORD(0, 0, 0);
//        lpDDSScape->Blt(&rcRectdes, nullptr, nullptr, DDBLT_COLORFILL, &ddbltfx);

        for (short y = 0; y < MAXYKACH; y++)
            for (short x = 0; x < MAXXKACH; x++)
            {
                if (!Scape[x][y].Entdeckt) continue; // Nicht entdeckte Felder nicht malen

                rcRectdes.left = Scape[x][y].xScreen;
                rcRectdes.top = Scape[x][y].yScreen;
                rcRectdes.right = rcRectdes.left + KXPIXEL;
                rcRectdes.bottom = rcRectdes.top + KYPIXEL;
                if (Scape[x][y].Art == 4)
                {
                    rcRectsrc.left = KXPIXEL * Scape[x][y].Typ;
                    rcRectsrc.right = KXPIXEL * Scape[x][y].Typ + KXPIXEL;
                    rcRectsrc.top = 0;
                    rcRectsrc.bottom = KYPIXEL;
                }
                else if (Scape[x][y].Art == 0) // trockenes Land
                {
                    rcRectsrc.left = KXPIXEL * Scape[x][y].Typ;
                    rcRectsrc.right = KXPIXEL * Scape[x][y].Typ + KXPIXEL;
                    rcRectsrc.top = 4 * KYPIXEL;
                    rcRectsrc.bottom = 5 * KYPIXEL;
                }
                else
                {
                    if ((Scape[x][y].Typ == 0) && (Scape[x][y].Art == 1))
                    {
                        rcRectsrc.left = 0 * KXPIXEL;
                        rcRectsrc.top = 3 * KYPIXEL;
                        rcRectsrc.right = 1 * KXPIXEL;
                        rcRectsrc.bottom = 4 * KYPIXEL;
                    }
                    if ((Scape[x][y].Typ == 0) && (Scape[x][y].Art == 2))
                    {
                        rcRectsrc.left = 1 * KXPIXEL;
                        rcRectsrc.top = 3 * KYPIXEL;
                        rcRectsrc.right = 2 * KXPIXEL;
                        rcRectsrc.bottom = 4 * KYPIXEL;
                    }
                    if ((Scape[x][y].Typ == 0) && (Scape[x][y].Art == 3))
                    {
                        rcRectsrc.left = 2 * KXPIXEL;
                        rcRectsrc.top = 3 * KYPIXEL;
                        rcRectsrc.right = 3 * KXPIXEL;
                        rcRectsrc.bottom = 4 * KYPIXEL;
                    }
                }
                // Landschaftskacheln zeichnen
                Renderer::Blitten(lpDDSMisc, lpDDSScape, true);

                // Gitter drüberlegen
                if (Gitter)
                {
                    rcRectsrc.left = KXPIXEL * Scape[x][y].Typ;
                    rcRectsrc.right = KXPIXEL * Scape[x][y].Typ + KXPIXEL;
                    rcRectsrc.top = 1 * KYPIXEL;
                    rcRectsrc.bottom = 1 * KYPIXEL + KYPIXEL;
                    Renderer::Blitten(lpDDSMisc, lpDDSScape, true);
                }

                // Landschaftsobjekte zeichnen (falls Animationen ausgeschaltet sind)
                if ((!LAnimation) && (Scape[x][y].Objekt != -1))
                {
                    if ((Scape[x][y].Objekt >= MEERWELLEN) && (Scape[x][y].Objekt <= SCHLEUSE6))
                    {
                        rcRectsrc.left = Bmp[Scape[x][y].Objekt].rcSrc.left;
                        rcRectsrc.right = Bmp[Scape[x][y].Objekt].rcSrc.right;
                        if (Scape[x][y].Objekt == MEERWELLEN)
                        {
                            short i = rand() % 6;
                            rcRectsrc.top = Bmp[Scape[x][y].Objekt].rcSrc.top + i * Bmp[Scape[x][y].Objekt].Hoehe;
                            rcRectsrc.bottom = Bmp[Scape[x][y].Objekt].rcSrc.bottom + i * Bmp[Scape[x][y].Objekt].Hoehe;
                        }
                        else
                        {
                            rcRectsrc.top = Bmp[Scape[x][y].Objekt].rcSrc.top;
                            rcRectsrc.bottom = Bmp[Scape[x][y].Objekt].rcSrc.bottom;
                        }
                        rcRectdes.left = Scape[x][y].xScreen + Bmp[Scape[x][y].Objekt].rcDes.left;
                        rcRectdes.right = Scape[x][y].xScreen + Bmp[Scape[x][y].Objekt].rcDes.right;
                        rcRectdes.top = Scape[x][y].yScreen + Bmp[Scape[x][y].Objekt].rcDes.top;
                        rcRectdes.bottom = Scape[x][y].yScreen + Bmp[Scape[x][y].Objekt].rcDes.bottom;
                        // Landschaftsobjekt zeichnen
                        Renderer::Blitten(lpDDSAnimation, lpDDSScape, true);
                    }
                }

                // MiniMap zeichnen
                rcRectdes.left = 2 * x;
                rcRectdes.top = 2 * y;
                rcRectdes.right = rcRectdes.left + 2;
                rcRectdes.bottom = rcRectdes.top + 2;

                if ((Scape[x][y].Art == 1) && (Scape[x][y].Typ == 0)) // Meer
                    ddbltfx.dwFillColor = Renderer::RGB2DWORD(228, 207, 182);
                else
                {
                    if ((Scape[x][y].Typ == 0) &&
                        ((Scape[x][y].Art == 2) ||
                            (Scape[x][y].Art == 3))) // Strand
                        ddbltfx.dwFillColor = Renderer::RGB2DWORD(112, 103, 93);
                    else
                    // Land
                        ddbltfx.dwFillColor = Renderer::RGB2DWORD(139 + Scape[x][y].Hoehe * 20, 128 + Scape[x][y].Hoehe * 20, 115 + Scape[x][y].Hoehe * 20);
                }
//                lpDDSKarte->Blt(&rcRectdes, nullptr, nullptr, DDBLT_COLORFILL, &ddbltfx);
            }
    }

    void CheckSpzButton()
    {
        if ((Scape[Guy.Pos.x][Guy.Pos.y].Objekt >= FELD) && (Scape[Guy.Pos.x][Guy.Pos.y].Objekt <= FEUERSTELLE) &&
            (Scape[Guy.Pos.x][Guy.Pos.y].Phase >= Bmp[Scape[Guy.Pos.x][Guy.Pos.y].Objekt].Anzahl) &&
            (Bmp[BUTTSTOP].Phase == -1))
        {
            if (Bmp[BUTTWEITER].Phase == -1) Bmp[BUTTWEITER].Phase = 0;
        }
        else Bmp[BUTTWEITER].Phase = -1;

        if ((Bmp[BUTTSTOP].Phase == -1) && (((Scape[Guy.Pos.x][Guy.Pos.y].Objekt == BOOT) &&
                (Scape[Guy.Pos.x][Guy.Pos.y].Phase < Bmp[Scape[Guy.Pos.x][Guy.Pos.y].Objekt].Anzahl)) ||
            ((BootsFahrt) &&
                (((Scape[Guy.Pos.x - 1][Guy.Pos.y].Art != 1) && (Scape[Guy.Pos.x - 1][Guy.Pos.y].Objekt == -1)) ||
                    ((Scape[Guy.Pos.x][Guy.Pos.y - 1].Art != 1) && (Scape[Guy.Pos.x][Guy.Pos.y - 1].Objekt == -1)) ||
                    ((Scape[Guy.Pos.x + 1][Guy.Pos.y].Art != 1) && (Scape[Guy.Pos.x + 1][Guy.Pos.y].Objekt == -1)) ||
                    ((Scape[Guy.Pos.x][Guy.Pos.y + 1].Art != 1) && (Scape[Guy.Pos.x][Guy.Pos.y + 1].Objekt == -1))))))
        {
            if (Bmp[BUTTABLEGEN].Phase == -1) Bmp[BUTTABLEGEN].Phase = 0;
        }
        else Bmp[BUTTABLEGEN].Phase = -1;
    }

    bool CheckRohstoff()
    {
        short Benoetigt = 0; // Anzahl der Gesamtbenötigten Rohstoffe
        for (short i = 0; i < BILDANZ; i++) Benoetigt += Bmp[Scape[Guy.Pos.x][Guy.Pos.y].Objekt].Rohstoff[i];

        float GebrauchtTmp = Benoetigt / static_cast<float>(Bmp[Scape[Guy.Pos.x][Guy.Pos.y].Objekt].AkAnzahl); // Soviel Rohstoffe werden für diesen Schritt benötigt
        short Gebraucht = static_cast<short>(GebrauchtTmp * Scape[Guy.Pos.x][Guy.Pos.y].AkNummer -
            static_cast<short>(GebrauchtTmp * (Scape[Guy.Pos.x][Guy.Pos.y].AkNummer - 1))); // Soviel Rohstoffe werden für diesen Schritt benötigt


        while (true)
        {
            bool Check = false; // Wenn kein Rohstoff mehr vorhanden nur noch einmal die While-Schleife
            for (short i = 0; i < BILDANZ; i++)
            {
                if (Gebraucht == 0) return true;
                if ((Scape[Guy.Pos.x][Guy.Pos.y].Rohstoff[i] > 0) &&
                    (Guy.Inventar[i] > 0))
                {
                    Guy.Inventar[i]--;
                    Scape[Guy.Pos.x][Guy.Pos.y].Rohstoff[i]--;
                    Gebraucht--;
                    if (Gebraucht == 0) return true;
                    Check = true;
                }
            }
            if (Check == false) break;
        }
        PapierText = Renderer::DrawText(ROHSTOFFNICHT, TXTPAPIER, 1);
        Guy.AkNummer = 0;
        Guy.Aktion = Action::CANCEL;
        Bmp[BUTTSTOP].Phase = -1;
        return false;
    }

    void Compute(short MinGroesse, short MaxGroesse) // Groesse der Insel in Anzahl der Landkacheln
    {
        short i, j;
        bool gefunden;

        short Vierecke[13][4][13] = //0=Passt nicht 1=1runter 2=gleiche Hoehe 3=1hoch
        {//		  0	  1	  2	  3	  4	  5	  6	  7	  8	  9	 10	 11	 12		
            {{{2}, {0}, {1}, {0}, {2}, {2}, {0}, {0}, {2}, {0}, {0}, {1}, {1}},//0
                {{2}, {1}, {0}, {2}, {0}, {0}, {0}, {2}, {2}, {0}, {1}, {1}, {0}},//1
                /*0*/{{2}, {0}, {2}, {0}, {1}, {0}, {2}, {2}, {0}, {1}, {1}, {0}, {0}},//2 Kante
                {{2}, {2}, {0}, {1}, {0}, {2}, {2}, {0}, {0}, {1}, {0}, {0}, {1}} //3
            },
            {{{0}, {2}, {0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {0}},
                {{2}, {1}, {0}, {2}, {0}, {0}, {0}, {2}, {2}, {0}, {1}, {1}, {0}},
                /*1*/{{0}, {2}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {0}, {0}, {2}, {0}},
                {{3}, {3}, {0}, {2}, {0}, {3}, {3}, {0}, {0}, {2}, {0}, {0}, {2}}
            },
            {{{2}, {0}, {1}, {0}, {2}, {2}, {0}, {0}, {2}, {0}, {0}, {1}, {1}},
                {{0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {0}, {0}, {2}},
                /*2*/{{3}, {0}, {3}, {0}, {2}, {0}, {3}, {3}, {0}, {2}, {2}, {0}, {0}},
                {{0}, {0}, {2}, {0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}}
            },
            {{{0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {2}, {0}, {0}, {0}},
                {{3}, {2}, {0}, {3}, {0}, {0}, {0}, {3}, {3}, {0}, {2}, {2}, {0}},
                /*3*/{{0}, {0}, {0}, {2}, {0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}},
                {{2}, {2}, {0}, {1}, {0}, {0}, {2}, {0}, {0}, {1}, {0}, {0}, {1}}
            },
            {{{3}, {0}, {2}, {0}, {3}, {3}, {0}, {0}, {3}, {0}, {0}, {2}, {2}},
                {{0}, {0}, {0}, {0}, {2}, {2}, {0}, {0}, {0}, {2}, {0}, {0}, {0}},
                /*4*/{{2}, {0}, {2}, {0}, {0}, {0}, {2}, {2}, {0}, {1}, {1}, {0}, {0}},
                {{0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {2}, {0}, {0}}
            },//  0	  1	  2	  3	  4	  5	  6	  7	  8	  9	 10	 11	 12	
            {{{0}, {2}, {0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {0}},
                {{2}, {1}, {0}, {2}, {0}, {0}, {0}, {2}, {2}, {0}, {1}, {1}, {0}},
                /*5*/{{2}, {0}, {2}, {0}, {1}, {0}, {2}, {2}, {0}, {1}, {1}, {0}, {0}},
                {{0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {2}, {0}, {0}}
            },
            {{{2}, {0}, {1}, {0}, {2}, {2}, {0}, {0}, {2}, {0}, {0}, {1}, {1}},
                {{2}, {1}, {0}, {2}, {0}, {0}, {0}, {2}, {2}, {0}, {1}, {1}, {0}},
                /*6*/{{0}, {2}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {0}, {0}, {2}, {0}},
                {{0}, {0}, {2}, {0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}}
            },
            {{{2}, {0}, {1}, {0}, {2}, {2}, {0}, {0}, {2}, {0}, {0}, {1}, {1}},
                {{0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {0}, {0}, {2}},
                /*7*/{{0}, {0}, {0}, {2}, {0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}},
                {{2}, {2}, {0}, {1}, {0}, {2}, {2}, {0}, {0}, {1}, {0}, {0}, {1}}
            },
            {{{0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {2}, {0}, {0}, {0}},
                {{0}, {0}, {0}, {0}, {2}, {2}, {0}, {0}, {0}, {2}, {0}, {0}, {0}},
                /*8*/{{2}, {0}, {2}, {0}, {1}, {0}, {2}, {2}, {0}, {1}, {1}, {0}, {0}},
                {{2}, {2}, {0}, {1}, {0}, {2}, {2}, {0}, {0}, {1}, {0}, {0}, {1}}
            },
            {{{3}, {0}, {2}, {0}, {3}, {3}, {0}, {0}, {3}, {0}, {0}, {2}, {2}},
                {{3}, {2}, {0}, {3}, {0}, {0}, {0}, {3}, {3}, {0}, {2}, {2}, {0}},
                /*9*/{{0}, {0}, {0}, {2}, {0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}},
                {{0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {2}, {0}, {0}}
            },//  0	  1	  2	  3	  4	  5	  6	  7	  8	  9	 10	 11	 12	
            {{{3}, {0}, {2}, {0}, {3}, {3}, {0}, {0}, {3}, {0}, {0}, {2}, {2}},
                {{0}, {0}, {0}, {0}, {2}, {2}, {0}, {0}, {0}, {2}, {0}, {0}, {0}},
                /*10*/{{0}, {2}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {0}, {0}, {2}, {0}},
                {{3}, {3}, {0}, {2}, {0}, {3}, {3}, {0}, {0}, {2}, {0}, {0}, {2}}
            },
            {{{0}, {2}, {0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {0}},
                {{0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {0}, {0}, {2}},
                /*11*/{{3}, {0}, {3}, {0}, {2}, {0}, {3}, {3}, {0}, {2}, {2}, {0}, {0}},
                {{3}, {3}, {0}, {2}, {0}, {3}, {3}, {0}, {0}, {2}, {0}, {0}, {2}}
            },
            {{{0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}, {2}, {0}, {0}, {0}},
                {{3}, {2}, {0}, {3}, {0}, {0}, {0}, {3}, {3}, {0}, {2}, {2}, {0}},
                /*12*/{{3}, {0}, {3}, {0}, {2}, {0}, {3}, {3}, {0}, {2}, {2}, {0}, {0}},
                {{0}, {0}, {2}, {0}, {0}, {0}, {0}, {2}, {0}, {0}, {0}, {2}, {0}}
            },
        };

        short Mittex = MAXXKACH / 2 - 1;
        short Mittey = MAXYKACH / 2 - 1;

        for (short m = 0; m < 1000; m++) // 100mal wiederholen, oder bis eine geeignete Insel gefunden ist
        {
            for (short y = 0; y < MAXYKACH; y++)
                for (short x = 0; x < MAXXKACH; x++)
                {
                    Scape[x][y].Typ = 0;
                    Scape[x][y].Art = 0;
                    Scape[x][y].Hoehe = 0;
                    Scape[x][y].Markiert = false;
                    Scape[x][y].Begehbar = true;
                    Scape[x][y].Entdeckt = false;
                    Scape[x][y].LaufZeit = 1;
                    Scape[x][y].Objekt = -1;
                    Scape[x][y].Reverse = false;
                    Scape[x][y].ObPos.x = 0;
                    Scape[x][y].ObPos.y = 0;
                    Scape[x][y].Phase = -1;
                    Scape[x][y].AkNummer = 0;
                    Scape[x][y].GPosAlt.x = 0;
                    Scape[x][y].GPosAlt.y = 0;
                    for (i = 0; i < BILDANZ; i++) Scape[x][y].Rohstoff[i] = 0;
                    Scape[x][y].Timer = 0;
                }

            short x; // Startposition der Berechnung
            short y;
            Scape[Mittex][Mittey].Typ = 0; // Gipfel festlegen (Flach)	
            Scape[Mittex][Mittey].Hoehe = GIPFEL; // und mit der Hoehe 

            // Spiralring von Innen aus gezählt
            for (short l = 0; l <= Mittey - 1; l++)
            {
                if (l >= Mittex) break;
                y = Mittey - l - 1;
                // Als erstes den oberen Bereich von links nach rechts durchgehen 
                for (x = Mittex - l; x <= Mittex + l; x++)
                {
                    gefunden = false;
                    i = 0;
                    while (!gefunden) // Passendes Teil finden und Hoehe festlegen
                    {
                        i += 1;
                        if (i == 1000) { gefunden = true; }

                        Scape[x][y].Typ = rand() % 13;
                        for (j = 0; j < 10; j++)
                        {
                            if (!((Scape[x][y].Typ == 0) || (Scape[x][y].Typ == 1)))
                            {
                                Scape[x][y].Typ = rand() % 13;
                            }
                        }

                        if ((x == Mittex - l) || ((x != Mittex - l)
                            && ((Vierecke[Scape[x][y + 1].Typ][1][Scape[x][y].Typ] != 0)
                                && (Vierecke[Scape[x - 1][y].Typ][2][Scape[x][y].Typ] != 0))))
                        {
                            if (Vierecke[Scape[x][y + 1].Typ][1][Scape[x][y].Typ] == 1)
                            {
                                Scape[x][y].Hoehe = Scape[x][y + 1].Hoehe - 1;
                                if (Scape[x][y].Hoehe < 0)
                                {
                                    Scape[x][y].Typ = 0;
                                    Scape[x][y].Hoehe = 0;
                                }
                                gefunden = true;
                            }
                            if (Vierecke[Scape[x][y + 1].Typ][1][Scape[x][y].Typ] == 2)
                            {
                                Scape[x][y].Hoehe = Scape[x][y + 1].Hoehe;
                                gefunden = true;
                            }
                            if (Vierecke[Scape[x][y + 1].Typ][1][Scape[x][y].Typ] == 3)
                            {
                                Scape[x][y].Hoehe = Scape[x][y + 1].Hoehe + 1;
                                gefunden = true;
                            }
                        }
                        // Verzwickte Fälle ausfiltern
                        if (((Vierecke[Scape[x][y].Typ][2][3] == 2) && (Vierecke[Scape[x + 1][y + 1].Typ][1][4] == 2)) ||
                            ((Vierecke[Scape[x][y].Typ][2][1] == 2) && (Vierecke[Scape[x + 1][y + 1].Typ][1][2] == 2)))
                        {
                            gefunden = false;
                        }
                        // Nebeninseln vermeiden
                        if (((Scape[x - 1][y].Typ == 0) && (Scape[x - 1][y].Hoehe == 0)) &&
                            ((Scape[x][y + 1].Typ == 0) && (Scape[x][y + 1].Hoehe == 0)))
                        {
                            Scape[x][y].Typ = 0;
                            Scape[x][y].Hoehe = 0;
                        }
                    }
                }

                // Teil rechts-oben  
                x = Mittex + l + 1;
                y = Mittey - l - 1;
                gefunden = false;
                i = 0;
                while (!gefunden) // Passendes Teil finden und Hoehe festlegen
                {
                    i += 1;
                    if (i == 1000) { gefunden = true; }

                    Scape[x][y].Typ = rand() % 13;
                    for (j = 0; j < 10; j++)
                    {
                        if (!((Scape[x][y].Typ == 0) || (Scape[x][y].Typ == 5)))
                        {
                            Scape[x][y].Typ = rand() % 13;
                        }
                    }

                    if (Vierecke[Scape[x - 1][y].Typ][2][Scape[x][y].Typ] == 1)
                    {
                        Scape[x][y].Hoehe = Scape[x - 1][y].Hoehe - 1;
                        if (Scape[x][y].Hoehe < 0)
                        {
                            Scape[x][y].Typ = 0;
                            Scape[x][y].Hoehe = 0;
                        }
                        gefunden = true;
                    }
                    if (Vierecke[Scape[x - 1][y].Typ][2][Scape[x][y].Typ] == 2)
                    {
                        Scape[x][y].Hoehe = Scape[x - 1][y].Hoehe;
                        gefunden = true;
                    }
                    if (Vierecke[Scape[x - 1][y].Typ][2][Scape[x][y].Typ] == 3)
                    {
                        Scape[x][y].Hoehe = Scape[x - 1][y].Hoehe + 1;
                        gefunden = true;
                    }
                    // Verzwickte Fälle ausfiltern
                    if (((Vierecke[Scape[x][y].Typ][3][2] == 2) && (Vierecke[Scape[x - 1][y + 1].Typ][2][3] == 2)) ||
                        ((Vierecke[Scape[x][y].Typ][3][4] == 2) && (Vierecke[Scape[x - 1][y + 1].Typ][2][1] == 2)))
                    {
                        gefunden = false;
                    }
                    // Nebeninseln vermeiden
                    if ((Scape[x - 1][y].Typ == 0) && (Scape[x - 1][y].Hoehe == 0))
                    {
                        Scape[x][y].Typ = 0;
                        Scape[x][y].Hoehe = 0;
                    }
                }

                // Den rechten Bereich von oben nach unten durchgehen 
                x = Mittex + l + 1;
                for (y = Mittey - l; y <= Mittey + l; y++)
                {
                    gefunden = false;
                    i = 0;
                    while (!gefunden) // Passendes Teil finden und Hoehe festlegen
                    {
                        i += 1;
                        if (i == 1000) { gefunden = true; }

                        Scape[x][y].Typ = rand() % 13;
                        for (j = 0; j < 10; j++)
                        {
                            if (!((Scape[x][y].Typ == 0) || (Scape[x][y].Typ == 4)))
                            {
                                Scape[x][y].Typ = rand() % 13;
                            }
                        }

                        if ((Vierecke[Scape[x - 1][y].Typ][2][Scape[x][y].Typ] != 0)
                            && (Vierecke[Scape[x][y - 1].Typ][3][Scape[x][y].Typ] != 0))
                        {
                            if (Vierecke[Scape[x][y - 1].Typ][3][Scape[x][y].Typ] == 1)
                            {
                                Scape[x][y].Hoehe = Scape[x][y - 1].Hoehe - 1;
                                if (Scape[x][y].Hoehe < 0)
                                {
                                    Scape[x][y].Typ = 0;
                                    Scape[x][y].Hoehe = 0;
                                }
                                gefunden = true;
                            }
                            if (Vierecke[Scape[x][y - 1].Typ][3][Scape[x][y].Typ] == 2)
                            {
                                Scape[x][y].Hoehe = Scape[x][y - 1].Hoehe;
                                gefunden = true;
                            }
                            if (Vierecke[Scape[x][y - 1].Typ][3][Scape[x][y].Typ] == 3)
                            {
                                Scape[x][y].Hoehe = Scape[x][y - 1].Hoehe + 1;
                                gefunden = true;
                            }
                        }
                        // Verzwickte Fälle ausfiltern
                        if (((Vierecke[Scape[x][y].Typ][3][2] == 2) && (Vierecke[Scape[x - 1][y + 1].Typ][2][3] == 2)) ||
                            ((Vierecke[Scape[x][y].Typ][3][4] == 2) && (Vierecke[Scape[x - 1][y + 1].Typ][2][1] == 2)))
                        {
                            gefunden = false;
                        }
                        // Nebeninseln vermeiden
                        if (((Scape[x - 1][y].Typ == 0) && (Scape[x - 1][y].Hoehe == 0)) &&
                            ((Scape[x][y - 1].Typ == 0) && (Scape[x][y - 1].Hoehe == 0)))
                        {
                            Scape[x][y].Typ = 0;
                            Scape[x][y].Hoehe = 0;
                        }
                    }
                }

                // Teil rechts-unten  
                x = Mittex + l + 1;
                y = Mittey + l + 1;
                gefunden = false;
                i = 0;
                while (!gefunden) // Passendes Teil finden und Hoehe festlegen
                {
                    i += 1;
                    if (i == 1000) { gefunden = true; }

                    Scape[x][y].Typ = rand() % 13;
                    for (j = 0; j < 10; j++)
                    {
                        if (!((Scape[x][y].Typ == 0) || (Scape[x][y].Typ == 8)))
                        {
                            Scape[x][y].Typ = rand() % 13;
                        }
                    }

                    if (Vierecke[Scape[x][y - 1].Typ][3][Scape[x][y].Typ] == 1)
                    {
                        Scape[x][y].Hoehe = Scape[x][y - 1].Hoehe - 1;
                        if (Scape[x][y].Hoehe < 0)
                        {
                            Scape[x][y].Typ = 0;
                            Scape[x][y].Hoehe = 0;
                        }
                        gefunden = true;
                    }
                    if (Vierecke[Scape[x][y - 1].Typ][3][Scape[x][y].Typ] == 2)
                    {
                        Scape[x][y].Hoehe = Scape[x][y - 1].Hoehe;
                        gefunden = true;
                    }
                    if (Vierecke[Scape[x][y - 1].Typ][3][Scape[x][y].Typ] == 3)
                    {
                        Scape[x][y].Hoehe = Scape[x][y - 1].Hoehe + 1;
                        gefunden = true;
                    }
                    // Verzwickte Fälle ausfiltern
                    if (((Vierecke[Scape[x][y].Typ][0][1] == 2) && (Vierecke[Scape[x - 1][y - 1].Typ][3][2] == 2)) ||
                        ((Vierecke[Scape[x][y].Typ][0][3] == 2) && (Vierecke[Scape[x - 1][y - 1].Typ][3][4] == 2)))
                    {
                        gefunden = false;
                    }
                    // Nebeninsel vermeiden
                    if ((Scape[x][y - 1].Typ == 0) && (Scape[x][y - 1].Hoehe == 0))
                    {
                        Scape[x][y].Typ = 0;
                        Scape[x][y].Hoehe = 0;
                    }
                }

                // Den unteren Bereich von rechts nach links durchgehen 
                y = Mittey + l + 1;
                for (x = Mittex + l; x >= Mittex - l; x--)
                {
                    gefunden = false;
                    i = 0;
                    while (!gefunden) // Passendes Teil finden und Hoehe festlegen
                    {
                        i += 1;
                        if (i == 1000) { gefunden = true; }

                        Scape[x][y].Typ = rand() % 13;
                        for (j = 0; j < 10; j++)
                        {
                            if (!((Scape[x][y].Typ == 0) || (Scape[x][y].Typ == 3)))
                            {
                                Scape[x][y].Typ = rand() % 13;
                            }
                        }

                        if ((Vierecke[Scape[x][y - 1].Typ][3][Scape[x][y].Typ] != 0)
                            && (Vierecke[Scape[x + 1][y].Typ][0][Scape[x][y].Typ] != 0))
                        {
                            if (Vierecke[Scape[x][y - 1].Typ][3][Scape[x][y].Typ] == 1)
                            {
                                Scape[x][y].Hoehe = Scape[x][y - 1].Hoehe - 1;
                                if (Scape[x][y].Hoehe < 0)
                                {
                                    Scape[x][y].Typ = 0;
                                    Scape[x][y].Hoehe = 0;
                                }
                                gefunden = true;
                            }
                            if (Vierecke[Scape[x][y - 1].Typ][3][Scape[x][y].Typ] == 2)
                            {
                                Scape[x][y].Hoehe = Scape[x][y - 1].Hoehe;
                                gefunden = true;
                            }
                            if (Vierecke[Scape[x][y - 1].Typ][3][Scape[x][y].Typ] == 3)
                            {
                                Scape[x][y].Hoehe = Scape[x][y - 1].Hoehe + 1;
                                gefunden = true;
                            }
                        }
                        // Verzwickte Fälle ausfiltern
                        if (((Vierecke[Scape[x][y].Typ][0][1] == 2) && (Vierecke[Scape[x - 1][y - 1].Typ][3][2] == 2)) ||
                            ((Vierecke[Scape[x][y].Typ][0][3] == 2) && (Vierecke[Scape[x - 1][y - 1].Typ][3][4] == 2)))
                        {
                            gefunden = false;
                        }
                        // Nebeninseln vermeiden
                        if (((Scape[x + 1][y].Typ == 0) && (Scape[x + 1][y].Hoehe == 0)) &&
                            ((Scape[x][y - 1].Typ == 0) && (Scape[x][y - 1].Hoehe == 0)))
                        {
                            Scape[x][y].Typ = 0;
                            Scape[x][y].Hoehe = 0;
                        }
                    }
                }

                //Teil links-unten  
                x = Mittex - l - 1;
                y = Mittey + l + 1;
                gefunden = false;
                i = 0;
                while (!gefunden) // Passendes Teil finden und Hoehe festlegen
                {
                    i += 1;
                    if (i == 1000) { gefunden = true; }

                    Scape[x][y].Typ = rand() % 13;
                    for (j = 0; j < 10; j++)
                    {
                        if (!((Scape[x][y].Typ == 0) || (Scape[x][y].Typ == 7)))
                        {
                            Scape[x][y].Typ = rand() % 13;
                        }
                    }

                    if (Vierecke[Scape[x + 1][y].Typ][0][Scape[x][y].Typ] == 1)
                    {
                        Scape[x][y].Hoehe = Scape[x + 1][y].Hoehe - 1;
                        if (Scape[x][y].Hoehe < 0)
                        {
                            Scape[x][y].Typ = 0;
                            Scape[x][y].Hoehe = 0;
                        }
                        gefunden = true;
                    }
                    if (Vierecke[Scape[x + 1][y].Typ][0][Scape[x][y].Typ] == 2)
                    {
                        Scape[x][y].Hoehe = Scape[x + 1][y].Hoehe;
                        gefunden = true;
                    }
                    if (Vierecke[Scape[x + 1][y].Typ][0][Scape[x][y].Typ] == 3)
                    {
                        Scape[x][y].Hoehe = Scape[x + 1][y].Hoehe + 1;
                        gefunden = true;
                    }
                    // Verzwickte Fälle ausfiltern
                    if (((Vierecke[Scape[x][y].Typ][1][2] == 2) && (Vierecke[Scape[x + 1][y - 1].Typ][0][3] == 2)) ||
                        ((Vierecke[Scape[x][y].Typ][1][4] == 2) && (Vierecke[Scape[x + 1][y - 1].Typ][0][1] == 2)))
                    {
                        gefunden = false;
                    }
                    // Nebeninsel vermeiden
                    if ((Scape[x + 1][y].Typ == 0) && (Scape[x + 1][y].Hoehe == 0))
                    {
                        Scape[x][y].Typ = 0;
                        Scape[x][y].Hoehe = 0;
                    }
                }

                // Den linken Bereich von unten nach oben durchgehen 
                x = Mittex - l - 1;
                for (y = Mittey + l; y >= Mittey - l - 1; y--)
                {
                    gefunden = false;
                    i = 0;
                    while (!gefunden) // Passendes Teil finden und Hoehe festlegen
                    {
                        i += 1;
                        if (i == 1000) { gefunden = true; }
                        Scape[x][y].Typ = rand() % 13;
                        for (j = 0; j < 10; j++)
                        {
                            if (!((Scape[x][y].Typ == 0) || (Scape[x][y].Typ == 2)))
                            {
                                Scape[x][y].Typ = rand() % 13;
                            }
                        }

                        if ((Vierecke[Scape[x + 1][y].Typ][0][Scape[x][y].Typ] != 0)
                            && (Vierecke[Scape[x][y + 1].Typ][1][Scape[x][y].Typ] != 0))
                        {
                            if (Vierecke[Scape[x][y + 1].Typ][1][Scape[x][y].Typ] == 1)
                            {
                                Scape[x][y].Hoehe = Scape[x][y + 1].Hoehe - 1;
                                if (Scape[x][y].Hoehe < 0)
                                {
                                    Scape[x][y].Typ = 0;
                                    Scape[x][y].Hoehe = 0;
                                }
                                gefunden = true;
                            }
                            if (Vierecke[Scape[x][y + 1].Typ][1][Scape[x][y].Typ] == 2)
                            {
                                Scape[x][y].Hoehe = Scape[x][y + 1].Hoehe;
                                gefunden = true;
                            }
                            if (Vierecke[Scape[x][y + 1].Typ][1][Scape[x][y].Typ] == 3)
                            {
                                Scape[x][y].Hoehe = Scape[x][y + 1].Hoehe + 1;
                                gefunden = true;
                            }
                        }
                        // Verzwickte Fälle ausfiltern
                        if (((Vierecke[Scape[x][y].Typ][1][2] == 2) && (Vierecke[Scape[x + 1][y - 1].Typ][0][3] == 2)) ||
                            ((Vierecke[Scape[x][y].Typ][1][4] == 2) && (Vierecke[Scape[x + 1][y - 1].Typ][0][1] == 2)))
                        {
                            gefunden = false;
                        }
                        // Nebeninseln vermeiden
                        if (((Scape[x + 1][y].Typ == 0) && (Scape[x + 1][y].Hoehe == 0)) &&
                            ((Scape[x][y + 1].Typ == 0) && (Scape[x][y + 1].Hoehe == 0)))
                        {
                            Scape[x][y].Typ = 0;
                            Scape[x][y].Hoehe = 0;
                        }
                    }
                }
            }
            short Anzahl = 0; // Anzahl der Landstücke
            bool CheckRand = true; // Reicht die Insel bis zum Rand?
            for (y = 0; y < MAXYKACH; y++) // Landfläche zählen
                for (x = 0; x < MAXXKACH; x++)
                {
                    if (Scape[x][y].Hoehe > 0) Anzahl++;

                    if (Scape[x][y].Typ == 0) Scape[x][y].LaufZeit = 1;
                    else Scape[x][y].LaufZeit = 2;

                    if ((Scape[x][y].Typ != 0) &&
                        ((x <= 2) || (x >= MAXXKACH - 2) || (y <= 2) || (y >= MAXYKACH - 2)))
                        CheckRand = false;
                }
            if ((Anzahl > MinGroesse) && (Anzahl < MaxGroesse) && (CheckRand)) break;
        }
    }

    void Meer() // Das Meer und den Strand bestimmen
    {
        short x, y;

        for (y = 0; y < MAXYKACH; y++) // Meer rausfinden
            for (x = 0; x < MAXXKACH; x++)
            {
                if ((Scape[x][y].Hoehe < 0) ||
                    ((Scape[x][y].Hoehe == 0) && (Scape[x][y].Typ == 0)))
                {
                    Scape[x][y].Typ = 0;
                    Scape[x][y].Hoehe = 0;
                    Scape[x][y].Art = 1;
                    Scape[x][y].Objekt = MEERWELLEN;
                    Scape[x][y].ObPos.x = static_cast<short>(Bmp[MEERWELLEN].rcDes.left);
                    Scape[x][y].ObPos.y = static_cast<short>(Bmp[MEERWELLEN].rcDes.top);
                    if (rand() % 2 == 0) Scape[x][y].Reverse = true;
                    Scape[x][y].Begehbar = false;
                    Scape[x][y].Phase = static_cast<float>(Bmp[Scape[x][y].Objekt].Anzahl -
                        rand() % (Bmp[Scape[x][y].Objekt].Anzahl) - 1);
                }
            }
        for (y = 1; y < MAXYKACH - 1; y++) // Strand rausfinden
            for (x = 1; x < MAXXKACH - 1; x++) // Alle Möglichkeiten durchgehen
            {
                if ((Scape[x][y].Typ == 0) && (Scape[x][y].Hoehe == 0))
                {
                    short Anzahl = 0; // Anzahl von angrenzenden Landstücken rausfinden
                    if (Scape[x - 1][y].Typ != 0) Anzahl++;
                    if (Scape[x - 1][y - 1].Typ != 0) Anzahl++;
                    if (Scape[x][y - 1].Typ != 0) Anzahl++;
                    if (Scape[x + 1][y - 1].Typ != 0) Anzahl++;
                    if (Scape[x + 1][y].Typ != 0) Anzahl++;
                    if (Scape[x + 1][y + 1].Typ != 0) Anzahl++;
                    if (Scape[x][y + 1].Typ != 0) Anzahl++;
                    if (Scape[x - 1][y + 1].Typ != 0) Anzahl++;

                    if ((Anzahl >= 1) && (Anzahl < 6))
                    {
                        Scape[x][y].Art = 2;
                        Scape[x][y].Objekt = -1;
                        Scape[x][y].Reverse = false;
                        Scape[x][y].Begehbar = true;
                        Scape[x][y].Phase = -1;
                        continue;
                    }
                    if (Anzahl >= 6)
                    {
                        Scape[x][y].Art = 3;
                        Scape[x][y].Objekt = -1;
                        Scape[x][y].Reverse = false;
                        Scape[x][y].Begehbar = false;
                        Scape[x][y].Phase = -1;
                        continue;
                    }
                    Scape[x][y].Art = 1; // sonst Meer
                    Scape[x][y].Objekt = MEERWELLEN;
                    Scape[x][y].ObPos.x = static_cast<short>(Bmp[MEERWELLEN].rcDes.left);
                    Scape[x][y].ObPos.y = static_cast<short>(Bmp[MEERWELLEN].rcDes.top);
                    Scape[x][y].Phase = static_cast<float>(Bmp[Scape[x][y].Objekt].Anzahl -
                        rand() % (Bmp[Scape[x][y].Objekt].Anzahl) - 1);
                    if (rand() % 2 == 0) Scape[x][y].Reverse = true;
                    Scape[x][y].Begehbar = false;
                }
            }
    }

    void ChangeBootsFahrt()
    {
        BootsFahrt = !BootsFahrt;
        // Begehbarkeit umdrehen
        for (short y = 0; y < MAXYKACH; y++)
            for (short x = 0; x < MAXXKACH; x++) Scape[x][y].Begehbar = !Scape[x][y].Begehbar;
    }

    void CheckRohr(short x, short y)
    {
        Scape[x][y].Phase = 1;
        if (Scape[x][y].Art == 0) Scape[x][y].Art = 4;
        if (Scape[x - 1][y].Art == 0) Scape[x - 1][y].Art = 4;
        if (Scape[x - 1][y - 1].Art == 0) Scape[x - 1][y - 1].Art = 4;
        if (Scape[x][y - 1].Art == 0) Scape[x][y - 1].Art = 4;
        if (Scape[x + 1][y - 1].Art == 0) Scape[x + 1][y - 1].Art = 4;
        if (Scape[x + 1][y].Art == 0) Scape[x + 1][y].Art = 4;
        if (Scape[x + 1][y + 1].Art == 0) Scape[x + 1][y + 1].Art = 4;
        if (Scape[x][y + 1].Art == 0) Scape[x][y + 1].Art = 4;
        if (Scape[x - 1][y + 1].Art == 0) Scape[x - 1][y + 1].Art = 4;

        if ((Scape[x - 1][y].Objekt == ROHR) && (Scape[x - 1][y].Phase == 0)) CheckRohr(x - 1, y);
        if ((Scape[x][y - 1].Objekt == ROHR) && (Scape[x][y - 1].Phase == 0)) CheckRohr(x, y - 1);
        if ((Scape[x + 1][y].Objekt == ROHR) && (Scape[x + 1][y].Phase == 0)) CheckRohr(x + 1, y);
        if ((Scape[x][y + 1].Objekt == ROHR) && (Scape[x][y + 1].Phase == 0)) CheckRohr(x, y + 1);
    }

    void FillRohr()
    {
        for (short y = 0; y < MAXYKACH; y++)
            for (short x = 0; x < MAXXKACH; x++)
            {
                if ((Scape[x][y].Objekt == ROHR) && (Scape[x][y].Phase < Bmp[ROHR].Anzahl))
                    Scape[x][y].Phase = 0;
                if (Scape[x][y].Art == 4) Scape[x][y].Art = 0;
                if ((Scape[x][y].Objekt >= SCHLEUSE1) && (Scape[x][y].Objekt <= SCHLEUSE6))
                {
                    Scape[x][y].Objekt -= 14;
                    Scape[x][y].ObPos.x = static_cast<short>(Bmp[Scape[x][y].Objekt].rcDes.left);
                    Scape[x][y].ObPos.y = static_cast<short>(Bmp[Scape[x][y].Objekt].rcDes.top);
                }
            }
        // StartRohr finden
        for (short y = 0; y < MAXYKACH; y++)
            for (short x = 0; x < MAXXKACH; x++)
            {
                if ((Scape[x][y].Objekt >= FLUSS1) && (Scape[x][y].Objekt <= SCHLEUSE6))
                {
                    if (Scape[x][y].Art == 0) Scape[x][y].Art = 4;
                    if (Scape[x - 1][y].Art == 0) Scape[x - 1][y].Art = 4;
                    if (Scape[x - 1][y - 1].Art == 0) Scape[x - 1][y - 1].Art = 4;
                    if (Scape[x][y - 1].Art == 0) Scape[x][y - 1].Art = 4;
                    if (Scape[x + 1][y - 1].Art == 0) Scape[x + 1][y - 1].Art = 4;
                    if (Scape[x + 1][y].Art == 0) Scape[x + 1][y].Art = 4;
                    if (Scape[x + 1][y + 1].Art == 0) Scape[x + 1][y + 1].Art = 4;
                    if (Scape[x][y + 1].Art == 0) Scape[x][y + 1].Art = 4;
                    if (Scape[x - 1][y + 1].Art == 0) Scape[x - 1][y + 1].Art = 4;
                }
                if ((Scape[x][y].Objekt != ROHR) || (Scape[x][y].Phase >= Bmp[ROHR].Anzahl))
                    continue;
                if ((Scape[x - 1][y].Objekt >= FLUSS5) && (Scape[x - 1][y].Objekt <= FLUSS10))
                {
                    Scape[x - 1][y].Objekt += 14;
                    Scape[x - 1][y].ObPos.x = static_cast<short>(Bmp[Scape[x - 1][y].Objekt].rcDes.left);
                    Scape[x - 1][y].ObPos.y = static_cast<short>(Bmp[Scape[x - 1][y].Objekt].rcDes.top);
                    CheckRohr(x, y);
                }
                else if ((Scape[x - 1][y].Objekt >= SCHLEUSE1) && (Scape[x - 1][y].Objekt <= SCHLEUSE6))
                {
                    CheckRohr(x, y);
                }
                if ((Scape[x][y - 1].Objekt >= FLUSS5) && (Scape[x][y - 1].Objekt <= FLUSS10))
                {
                    Scape[x][y - 1].Objekt += 14;
                    Scape[x][y - 1].ObPos.x = static_cast<short>(Bmp[Scape[x][y - 1].Objekt].rcDes.left);
                    Scape[x][y - 1].ObPos.y = static_cast<short>(Bmp[Scape[x][y - 1].Objekt].rcDes.top);
                    CheckRohr(x, y);
                }
                else if ((Scape[x][y - 1].Objekt >= SCHLEUSE1) && (Scape[x][y - 1].Objekt <= SCHLEUSE6))
                {
                    CheckRohr(x, y);
                }
                if ((Scape[x + 1][y].Objekt >= FLUSS5) && (Scape[x + 1][y].Objekt <= FLUSS10))
                {
                    Scape[x + 1][y].Objekt += 14;
                    Scape[x + 1][y].ObPos.x = static_cast<short>(Bmp[Scape[x + 1][y].Objekt].rcDes.left);
                    Scape[x + 1][y].ObPos.y = static_cast<short>(Bmp[Scape[x + 1][y].Objekt].rcDes.top);
                    CheckRohr(x, y);
                }
                else if ((Scape[x + 1][y].Objekt >= SCHLEUSE1) && (Scape[x + 1][y].Objekt <= SCHLEUSE6))
                {
                    CheckRohr(x, y);
                }
                if ((Scape[x][y + 1].Objekt >= FLUSS5) && (Scape[x][y + 1].Objekt <= FLUSS10))
                {
                    Scape[x][y + 1].Objekt += 14;
                    Scape[x][y + 1].ObPos.x = static_cast<short>(Bmp[Scape[x][y + 1].Objekt].rcDes.left);
                    Scape[x][y + 1].ObPos.y = static_cast<short>(Bmp[Scape[x][y + 1].Objekt].rcDes.top);
                    CheckRohr(x, y);
                }
                else if ((Scape[x][y + 1].Objekt >= SCHLEUSE1) && (Scape[x][y + 1].Objekt <= SCHLEUSE6))
                {
                    CheckRohr(x, y);
                }
            }
        // Felder auf trockenen Wiesen löschen
        for (short y = 0; y < MAXYKACH; y++)
            for (short x = 0; x < MAXXKACH; x++)
            {
                if ((Scape[x][y].Objekt == FELD) && (Scape[x][y].Art == 0))
                {
                    Scape[x][y].Objekt = -1;
                    Scape[x][y].ObPos.x = 0;
                    Scape[x][y].ObPos.y = 0;
                    Scape[x][y].Phase = -1;
                    Scape[x][y].AkNummer = 0;
                }
            }
        Generate();
    }

    bool CheckFluss(short x, short y) // Nachprüfen ob auf aktuellem Teil ein Fluss ist (Nur für die Fluss-Routine)
    {
        for (short i = 0; i < FLUSSANZAHL; i++)
            for (short j = 0; j < MAXFLUSS; j++)
            {
                if ((x == Flusslauf[i][j].x) && (y == Flusslauf[i][j].y)) return true;
            }
        return false;
    }

    void Fluss() // Anzahl der Flüsse und der minimale Länge
    {
        short i;
        short l = 0;
        short Richtung = 0; // Aktuelle Fliesrichtung von 0-3
        short x0 = 0;
        short y0 = 0;
        short x1 = 0;
        short y1 = 0;
        short x2 = 0; // x2,y2 Koordinate des zu prüfenden Teils
        short y2 = 0;
        bool gefunden = false;
        FLUSSLAUF Flusstmp[MAXFLUSS]; // Zum zwischenspeichern des Versuchs
        short Flusslaenge[FLUSSANZAHL];

        for (short m = 0; m < FLUSSANZAHL; m++)
            for (short j = 0; j < MAXFLUSS; j++)
            {
                Flusslauf[m][j].x = -1;
                Flusslauf[m][j].y = -1;
            }

        for (short m = 0; m < FLUSSANZAHL; m++)
        {
            bool fertig = false;
            short Laengetmp = 0;
            for (short j = 0; j < MAXFLUSS; j++)
            {
                Flusstmp[j].x = -1;
                Flusstmp[j].y = -1;
            }
            for (short k = 0; k < 1000; k++)
            {
                for (short o = 0; o < 10000; o++)
                {
                    gefunden = true;

                    x0 = rand() % MAXXKACH; // geeignete Quelle bestimmen
                    y0 = rand() % MAXYKACH;
                    if (CheckFluss(x0, y0)) gefunden = false;
                    if ((Scape[x0][y0].Typ != 0) ||
                        (Scape[x0][y0].Hoehe < 2))
                        gefunden = false;
                    if (gefunden) break;
                }
                if (!gefunden)
                {
                    MessageBeep(MB_OK);
                    break;
                } // Wenn keine Quelle mehr gefunden aufhören
                Flusslauf[m][0].x = x0;
                Flusslauf[m][0].y = y0;
                bool Strand = false;
                for (i = 1; i < MAXFLUSS; i++)
                {
                    gefunden = false;
                    if (!Strand) Richtung = rand() % 4; // Auf dem Strand geradeausfliessen
                    for (short l2 = 0; l2 < 4; l2++)
                    {
                        l = (Richtung + l2) % 4; // Im Urzeigersinn nachprüfen und bei der vorgegeben Richtung anfangen

                        x1 = Flusslauf[m][i - 1].x;
                        y1 = Flusslauf[m][i - 1].y;

                        if (l == 0)
                        {
                            x2 = x1 - 1;
                            y2 = y1;
                            if (((Scape[x1][y1].Typ == 0) || (Scape[x1][y1].Typ == 2)) &&
                                ((Scape[x2][y2].Typ == 0) || (Scape[x2][y2].Typ == 2)))
                            {
                                gefunden = (!CheckFluss(x2, y2));
                                if (gefunden) break;
                            }
                        }
                        if (l == 1)
                        {
                            x2 = x1;
                            y2 = y1 - 1;
                            if (((Scape[x1][y1].Typ == 0) || (Scape[x1][y1].Typ == 1)) &&
                                ((Scape[x2][y2].Typ == 0) || (Scape[x2][y2].Typ == 1)))
                            {
                                gefunden = (!CheckFluss(x2, y2));
                                if (gefunden) break;
                            }
                        }
                        if (l == 2)
                        {
                            x2 = x1 + 1;
                            y2 = y1;
                            if (((Scape[x1][y1].Typ == 0) || (Scape[x1][y1].Typ == 4)) &&
                                ((Scape[x2][y2].Typ == 0) || (Scape[x2][y2].Typ == 4)))
                            {
                                gefunden = (!CheckFluss(x2, y2));
                                if (gefunden) break;
                            }
                        }
                        if (l == 3)
                        {
                            x2 = x1;
                            y2 = y1 + 1;
                            if (((Scape[x1][y1].Typ == 0) || (Scape[x1][y1].Typ == 3)) &&
                                ((Scape[x2][y2].Typ == 0) || (Scape[x2][y2].Typ == 3)))
                            {
                                gefunden = (!CheckFluss(x2, y2));
                                if (gefunden) break;
                            }
                        }
                    }
                    if (!gefunden) break;

                    Flusslauf[m][i].x = x2;
                    Flusslauf[m][i].y = y2;
                    Richtung = l;

                    if ((Scape[Flusslauf[m][i].x][Flusslauf[m][i].y].Art == 2) && // Auf Strand die Richtung beibehalten
                        (Scape[Flusslauf[m][i].x][Flusslauf[m][i].y].Typ == 0))
                    {
                        if (Strand == true) break; // Nur ein Strandstück überfliessen
                        Strand = true;
                    }

                    if ((Scape[Flusslauf[m][i].x][Flusslauf[m][i].y].Art == 1) && // im meer aufhören
                        (Scape[Flusslauf[m][i].x][Flusslauf[m][i].y].Typ == 0))
                    {
                        fertig = true;
                        break;
                    }
                }
                if (fertig)
                {
                    if (i > Laengetmp)
                    {
                        // neue Variante speichern
                        if (gefunden) Laengetmp = i;
                        else Laengetmp = i - 1;
                        for (short j = 0; j <= Laengetmp; j++)
                        {
                            Flusstmp[j].x = Flusslauf[m][j].x;
                            Flusstmp[j].y = Flusslauf[m][j].y;
                        }
                    }
                }
                for (i = 0; i < MAXFLUSS; i++)
                {
                    Flusslauf[m][i].x = -1;
                    Flusslauf[m][i].y = -1;
                }
                fertig = false;
            }
            // Den längsten Fluss nehmen
            Flusslaenge[m] = Laengetmp;
            for (short j = 0; j <= Flusslaenge[m]; j++)
            {
                Flusslauf[m][j].x = Flusstmp[j].x;
                Flusslauf[m][j].y = Flusstmp[j].y;
            }

            // Die richtigen Wasserkacheln auswählen
            x0 , y0 , x1 , y1 , x2 , y2 = -1;
            for (m = 0; m < FLUSSANZAHL; m++)
            {
                for (i = 0; i <= Flusslaenge[m]; i++)
                {
                    // Für die Kachel, einen Vorgang davor
                    Scape[x1][y1].ObPos.x = static_cast<short>(Bmp[Scape[x1][y1].Objekt].rcDes.left);
                    Scape[x1][y1].ObPos.y = static_cast<short>(Bmp[Scape[x1][y1].Objekt].rcDes.top);

                    x1 = Flusslauf[m][i].x;
                    y1 = Flusslauf[m][i].y;

                    Scape[x1][y1].Phase = 0;

                    if (Scape[x1][y1].Art == 0) Scape[x1][y1].Art = 4;
                    if (Scape[x1 - 1][y1].Art == 0) Scape[x1 - 1][y1].Art = 4;
                    if (Scape[x1 - 1][y1 - 1].Art == 0) Scape[x1 - 1][y1 - 1].Art = 4;
                    if (Scape[x1][y1 - 1].Art == 0) Scape[x1][y1 - 1].Art = 4;
                    if (Scape[x1 + 1][y1 - 1].Art == 0) Scape[x1 + 1][y1 - 1].Art = 4;
                    if (Scape[x1 + 1][y1].Art == 0) Scape[x1 + 1][y1].Art = 4;
                    if (Scape[x1 + 1][y1 + 1].Art == 0) Scape[x1 + 1][y1 + 1].Art = 4;
                    if (Scape[x1][y1 + 1].Art == 0) Scape[x1][y1 + 1].Art = 4;
                    if (Scape[x1 - 1][y1 + 1].Art == 0) Scape[x1 - 1][y1 + 1].Art = 4;

                    if (i < Flusslaenge[m])
                    {
                        x2 = Flusslauf[m][i + 1].x;
                        y2 = Flusslauf[m][i + 1].y;
                    }

                    if (i != 0)
                    {
                        x0 = Flusslauf[m][i - 1].x;
                        y0 = Flusslauf[m][i - 1].y;
                    }
                    else
                    {
                        // Quellen
                        if (x2 > x1)
                        {
                            Scape[x1][y1].Objekt = QUELLE1;
                            continue;
                        }
                        if (x2 < x1)
                        {
                            Scape[x1][y1].Objekt = QUELLE4;
                            continue;
                        }
                        if (y2 > y1)
                        {
                            Scape[x1][y1].Objekt = QUELLE2;
                            continue;
                        }
                        if (y2 < y1)
                        {
                            Scape[x1][y1].Objekt = QUELLE3;
                            continue;
                        }
                    }

                    // Alle Möglichkeiten durchgehen

                    if (Scape[x1][y1].Typ == 1) Scape[x1][y1].Objekt = FLUSS1;
                    if (Scape[x1][y1].Typ == 2) Scape[x1][y1].Objekt = FLUSS2;
                    if (Scape[x1][y1].Typ == 3) Scape[x1][y1].Objekt = FLUSS3;
                    if (Scape[x1][y1].Typ == 4) Scape[x1][y1].Objekt = FLUSS4;

                    if (Scape[x1][y1].Typ == 0)
                    {
                        if ((x0 < x1) && (y0 == y1))
                        {
                            if (Scape[x1][y1].Art == 2) Scape[x1][y1].Objekt = MUENDUNG3; // Mündung
                            else
                            {
                                if ((x1 < x2) && (y1 == y2)) Scape[x1][y1].Objekt = FLUSS5;
                                if ((x1 == x2) && (y1 < y2)) Scape[x1][y1].Objekt = FLUSS7;
                                if ((x1 == x2) && (y1 > y2)) Scape[x1][y1].Objekt = FLUSS9;
                            }
                        }
                        if ((x0 == x1) && (y0 < y1))
                        {
                            if (Scape[x1][y1].Art == 2) Scape[x1][y1].Objekt = MUENDUNG4; // Mündung
                            else
                            {
                                if ((x1 < x2) && (y1 == y2)) Scape[x1][y1].Objekt = FLUSS8;
                                if ((x1 == x2) && (y1 < y2)) Scape[x1][y1].Objekt = FLUSS6;
                                if ((x1 > x2) && (y1 == y2))
                                {
                                    Scape[x1][y1].Objekt = FLUSS9;
                                    Scape[x1][y1].Reverse = true;
                                }
                            }
                        }
                        if ((x0 > x1) && (y0 == y1))
                        {
                            if (Scape[x1][y1].Art == 2) Scape[x1][y1].Objekt = MUENDUNG1; // Mündung
                            else
                            {
                                if ((x1 > x2) && (y1 == y2)) Scape[x1][y1].Objekt = FLUSS5;
                                if ((x1 == x2) && (y1 < y2)) Scape[x1][y1].Objekt = FLUSS10;
                                if ((x1 == x2) && (y1 > y2)) Scape[x1][y1].Objekt = FLUSS8;
                                Scape[x1][y1].Reverse = true;
                            }
                        }
                        if ((x0 == x1) && (y0 > y1))
                        {
                            if (Scape[x1][y1].Art == 2) Scape[x1][y1].Objekt = MUENDUNG2; // Mündung
                            else
                            {
                                if ((x1 == x2) && (y1 > y2))
                                    Scape[x1][y1].Objekt = FLUSS6;

                                if ((x1 > x2) && (y1 == y2))
                                    Scape[x1][y1].Objekt = FLUSS7;

                                Scape[x1][y1].Reverse = true;
                                if ((x1 < x2) && (y1 == y2))
                                {
                                    Scape[x1][y1].Objekt = FLUSS10;
                                    Scape[x1][y1].Reverse = false;
                                }
                            }
                        }
                    }
                }
                // Für das letzte Flussstück
                Scape[x1][y1].ObPos.x = static_cast<short>(Bmp[Scape[x1][y1].Objekt].rcDes.left);
                Scape[x1][y1].ObPos.y = static_cast<short>(Bmp[Scape[x1][y1].Objekt].rcDes.top);
            }
        }
    }

    void Baeume(short Prozent)
    {
        ZWEID Pos; // Da steht der Baum
        bool einGrosserBaum = false; // gibt es bereits einen großen Baum

        for (short y = 0; y < MAXYKACH; y++)//Alle Kacheln durchgehen
            for (short x = 0; x < MAXXKACH; x++)
            {
                if ((Scape[x][y].Objekt != -1) || ((Scape[x][y].Art == 3) && (Scape[x][y].Typ == 0)))
                    continue;

                // Wenn schon ein Objekt da ist oder Treibsand ist, dann mit nächsten Teil weitermachen
                if (rand() % (100 / Prozent) != 0) // Die Wahrscheinlichkeit für einen Baum bestimmen
                    continue;

                while (true)
                {
                    Pos.x = rand() % KXPIXEL;
                    Pos.y = rand() % KYPIXEL;
                    ZWEID Erg = Renderer::GetKachel(Scape[x][y].xScreen + Pos.x, Scape[x][y].yScreen + Pos.y);
                    if ((Erg.x == x) && (Erg.y == y)) break;
                }
                if ((Scape[x][y].Art == 2) && (Scape[x][y].Typ == 0)) // Bei Strand nur Palmen nehmen
                {
                    Scape[x][y].Objekt = BAUM2;
                }
                else
                {
                    short r = rand() % 5; // random speicherung
                    Scape[x][y].Objekt = BAUM1 + r;
                    if ((rand() % 50 == 1) || (!einGrosserBaum))
                    {
                        Scape[x][y].Objekt = BAUMGROSS;
                        einGrosserBaum = true;
                    }
                }
                // Linke obere Ecke speichern
                Scape[x][y].ObPos.x = Pos.x - static_cast<short>(Bmp[Scape[x][y].Objekt].Breite) / 2;
                Scape[x][y].ObPos.y = Pos.y - static_cast<short>(Bmp[Scape[x][y].Objekt].Hoehe);
                // Startphase
                if (Scape[x][y].Objekt == BUSCH)
                    Scape[x][y].Phase = static_cast<float>(Bmp[Scape[x][y].Objekt].Anzahl) - 1;
                else
                    Scape[x][y].Phase = static_cast<float>(Bmp[Scape[x][y].Objekt].Anzahl -
                        rand() % (Bmp[Scape[x][y].Objekt].Anzahl) - 1);
            }
    }

    void Piratenwrack()
    {
        short x = 0;
        short y = 0;

        short Richtung = rand() % 3;
        switch (Richtung)
        {
        case 0:
            x = MAXXKACH / 2;
            for (short i = 0; i < MAXYKACH; i++)
            {
                if (Scape[x][i].Art != 1)
                {
                    y = i - 1;
                    break;
                }
            }
            break;
        case 1:
            y = MAXYKACH / 2;
            for (short i = MAXXKACH - 1; i >= 0; i--)
            {
                if (Scape[i][y].Art != 1)
                {
                    x = i + 1;
                    break;
                }
            }
            break;
        case 2:
            x = MAXXKACH / 2;
            for (short i = MAXYKACH - 1; i >= 0; i--)
            {
                if (Scape[x][i].Art != 1)
                {
                    y = i + 1;
                    break;
                }
            }
            break;
        }
        Scape[x][y].Objekt = WRACK2;
        Scape[x][y].ObPos.x = static_cast<short>(Bmp[WRACK2].rcDes.left);
        Scape[x][y].ObPos.y = static_cast<short>(Bmp[WRACK2].rcDes.top);
    }

    void Schatz()
    {
        while (true)
        {
            // Diese Kachel wird angeschaut
            short x = rand() % (MAXXKACH - 1);
            short y = rand() % (MAXYKACH - 1);

            // nur auf flachen Kacheln ohne Objekt
            if ((Scape[x][y].Objekt == -1) && (Scape[x][y].Typ == 0) && (Scape[x][y].Art != 3))
            {
                if (SchatzPos.x == -1)
                {
                    SchatzPos.x = x;
                    SchatzPos.y = y;
                }

                lpDDSScape->Lock(nullptr, &ddsd, DDLOCK_WAIT, nullptr);
                lpDDSSchatzkarte->Lock(nullptr, &ddsd2, DDLOCK_WAIT, nullptr);

                for (short i = 0; i < SKARTEX; i++)
                    for (short j = 0; j < SKARTEY; j++)
                    {
                        Renderer::GetPixel(static_cast<short>(i + Scape[SchatzPos.x][SchatzPos.y].xScreen - SKARTEX / 2 + KXPIXEL / 2),
                                           static_cast<short>(j + Scape[SchatzPos.x][SchatzPos.y].yScreen - SKARTEY / 2 + 30), &ddsd);
                        Renderer::PutPixel(i, j, Renderer::RGB2DWORD((rgbStruct.r * 30 + rgbStruct.g * 59 + rgbStruct.b * 11) / 100,
                                                                     (rgbStruct.r * 30 + rgbStruct.g * 59 + rgbStruct.b * 11) / 100,
                                                                     (rgbStruct.r * 30 + rgbStruct.g * 59 + rgbStruct.b * 11) / 100 * 3 / 4), &ddsd2);
                    }

                lpDDSScape->Unlock(nullptr);
                lpDDSSchatzkarte->Unlock(nullptr);

                rcRectsrc = Bmp[KREUZ].rcSrc;
                rcRectdes.left = SKARTEX / 2 - Bmp[KREUZ].Breite / 2;
                rcRectdes.right = rcRectdes.left + Bmp[KREUZ].Breite;
                rcRectdes.top = SKARTEY / 2 - Bmp[KREUZ].Hoehe / 2;
                rcRectdes.bottom = rcRectdes.top + Bmp[KREUZ].Hoehe;
                Renderer::Blitten(Bmp[KREUZ].Surface, lpDDSSchatzkarte, true);

                lpDDSSchatzkarte->Lock(nullptr, &ddsd2, DDLOCK_WAIT, nullptr);

                // Weichzeichnen
                for (short i = 0; i < SKARTEX; i++)
                    for (short j = 0; j < SKARTEY; j++)
                    {
                        if ((i > 0) && (i < SKARTEX - 1) && (j > 0) && (j < SKARTEY - 1))
                        {
                            Renderer::GetPixel(i - 1, j, &ddsd2);
                            RGBSTRUCT rgbleft = rgbStruct;
                            Renderer::GetPixel(i, j - 1, &ddsd2);
                            RGBSTRUCT rgbtop = rgbStruct;
                            Renderer::GetPixel(i + 1, j, &ddsd2);
                            RGBSTRUCT rgbright = rgbStruct;
                            Renderer::GetPixel(i, j + 1, &ddsd2);
                            RGBSTRUCT rgbbottom = rgbStruct;
                            Renderer::GetPixel(i, j, &ddsd2);
                            Renderer::PutPixel(i, j, Renderer::RGB2DWORD(
                                                   (rgbleft.r + rgbtop.r + rgbright.r + rgbbottom.r + rgbStruct.r) / 5,
                                                   (rgbleft.g + rgbtop.g + rgbright.g + rgbbottom.g + rgbStruct.g) / 5,
                                                   (rgbleft.b + rgbtop.b + rgbright.b + rgbbottom.b + rgbStruct.b) / 5),
                                               &ddsd2);
                        }
                    }
                lpDDSSchatzkarte->Unlock(nullptr);
                break;
            }
        }
    }

    void CheckBenutze(short Objekt)
    {
        if (((Objekt == ROHSTEIN) && (TwoClicks == ROHAST)) || ((Objekt == ROHAST) && (TwoClicks == ROHSTEIN)))
        {
            if (Guy.Inventar[ROHAXT] < 1)
            {
                Guy.Inventar[ROHSTEIN]--;
                Guy.Inventar[ROHAST]--;
                Guy.Inventar[ROHAXT] = 1;
                Bmp[BUTTFAELLEN].Phase = 0;
                Bmp[BUTTBOOT].Phase = 0;
                Bmp[BUTTROHR].Phase = 0;
                PapierText = Renderer::DrawText(BAUEAXT, TXTPAPIER, 1);
                PlaySound(Sound::INVENTION, 100);
            }
            else if (Guy.Inventar[ROHEGGE] < 1)
            {
                Guy.Inventar[ROHSTEIN]--;
                Guy.Inventar[ROHAST]--;
                Guy.Inventar[ROHEGGE] = 1;
                Bmp[BUTTFELD].Phase = 0;
                PapierText = Renderer::DrawText(BAUEEGGE, TXTPAPIER, 1);
                PlaySound(Sound::INVENTION, 100);
            }
            else
            {
                PapierText = Renderer::DrawText(STEINPLUSASTNICHTS, TXTPAPIER, 1);
            }
        }
        else if (((Objekt == ROHLIANE) && (TwoClicks == ROHAST)) || ((Objekt == ROHAST) && (TwoClicks == ROHLIANE)))
        {
            if (Guy.Inventar[ROHANGEL] < 1)
            {
                Guy.Inventar[ROHLIANE]--;
                Guy.Inventar[ROHAST]--;
                Guy.Inventar[ROHANGEL] = 1;
                Bmp[BUTTANGELN].Phase = 0;
                PapierText = Renderer::DrawText(BAUEANGEL, TXTPAPIER, 1);
                PlaySound(Sound::INVENTION, 100);
            }
            else
            {
                PapierText = Renderer::DrawText(ASTPLUSLIANENICHTS, TXTPAPIER, 1);
            }
        }
        else if (((Objekt == ROHLIANE) && (TwoClicks == ROHSTEIN)) || ((Objekt == ROHSTEIN) && (TwoClicks == ROHLIANE)))
        {
            if (Guy.Inventar[ROHSCHLEUDER] < 1)
            {
                Guy.Inventar[ROHLIANE]--;
                Guy.Inventar[ROHSTEIN]--;
                Guy.Inventar[ROHSCHLEUDER] = 1;
                Bmp[BUTTSCHLEUDER].Phase = 0;
                PapierText = Renderer::DrawText(BAUESCHLEUDER, TXTPAPIER, 1);
                PlaySound(Sound::INVENTION, 100);
            }
            else
            {
                PapierText = Renderer::DrawText(STEINPLUSLIANENICHTS, TXTPAPIER, 1);
            }
        }
        else
        {
            PapierText = Renderer::DrawText(NICHTBASTELN, TXTPAPIER, 1);
        }
        TwoClicks = -1;
    }

    void Entdecken()
    {
        bool Aenderung = false;

        for (short i = -1; i <= 1; i++)
            for (short j = -1; j <= 1; j++)
            {
                if (!Scape[Guy.Pos.x + i][Guy.Pos.y + j].Entdeckt)
                {
                    Scape[Guy.Pos.x + i][Guy.Pos.y + j].Entdeckt = true;
                    Aenderung = true;
                }
            }

        if (Aenderung)
            Generate();
    }
} // namespace World
