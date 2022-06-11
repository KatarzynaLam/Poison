#pragma warning(disable: 4996)
#include <iostream>
#include <stdio.h>
#include<fstream>
#include <windows.h>
#include <time.h>

//definicja kolorow uzywanych do gry w sekwencji jakiej wystepuja na poczatku
#define K0 "green "
#define K1 "blue  "
#define K2 "red   "
#define K3 "violet"
#define K4 "yellow"
#define K5 "white "
#define K6 "black "
#define EKSPLOZJA 13            // eksplozja w przypadku wczytywaniu parametrow gry
#define MAKS_LICZBA_KOLOR 7 // liczba kolorow, lacznie z zielonym

using namespace std;

struct Dane {
    int liczbaGraczy;
    int liczbaKociolkow;                    //liczba kociołków odpowiadającą liczbie kolorów kart z wyłączeniem zielonego,
    int liczbaZielonych = 0;                //liczba zielonych kart w talii,
    int wartoscZielonych = (-1);    //wartość zielonych kart, poczatkowo ustawiona na -1
    int liczbaKart = 0;                             //liczba kart dla każdego innego koloru niż zielony.
    int liczbaWszystkichKart = 0;   //liczba wszystkich kart, łącznie z zielonymi
    int ekspozjaKociolka;                   //wartosc powyzej ktorej kociolek eksploduje
    int aktywnyGracz;                               //numer gracza aktywnego
    int indeksPierwszegoGrajacego;  //indeks gracza zaczynajacego gre jako pierwszy
};

struct Talia {
    int* wartosc; // wartosc karty
    int* kolor; // indeks koloru odpowiadajacy danej karcie
};

// struktura, która zawiera kolejne wartości kart, potasowane, które będą używane do
// wypisania danych na temat liczby różnych wartości zielonych kart oraz innych kolorów
struct Karty {
    int kolor;
    int liczbaKart = 0;
    int* wartosc;
};

struct Gracz {
    int* taliaKarty;                               // tutaj beda zapisywane tylko indeksy z talii 
                                                                    //(odpowiadajace indeksom kart ze struktury talia)
    int liczbaOdkrytychKart = 0;    // liczba kart w talii gracza
                                                                    //początkowo ustawiona na zero.
    int* zakryteKarty;                             // tutaj beda zapisywane tylko indeksy z talii 
                                                                    //(odpowiadajace indeksom kart ze struktury talia)
    int liczbaZakrytychKart = 0;    //liczba zakrytych kart - leżących przed graczem

};

struct WynikiGracza {
    int wynik = 0;                          //wynik danego gracza
    int* odpornosc;                        //indeksy kolorow na ktore jest odporny dany gracz
    int liczbaOdpornosci;           // liczba odpornosci danego gracza
    int* liczbaKart;                       // indeks w tablicy to bedzie indeks koloru
};

struct Kociolek {
    int* karty;                    //indeksy kart znajdujacych sie w kociolku
    int liczbaKart = 0;             //liczba kart znajdujaca sie w kociolku
    int kolor;                              // indeks koloru danego kociolka 
                                                    // (-1) to brak koloru
};

// Funkcja używana przy wczytywaniu stanu gry, służy do znalezienia indeksu koloru
//
//      Zwraca: indeks koloru
int zwrotIndeksuKoloru(char* kolor) {
    if (strcmp(kolor, K0) == 0) return 0;
    else if (strcmp(kolor, K1) == 0) return 1;
    else if (strcmp(kolor, K2) == 0) return 2;
    else if (strcmp(kolor, K3) == 0) return 3;
    else if (strcmp(kolor, K4) == 0) return 4;
    else if (strcmp(kolor, K5) == 0) return 5;
    else if (strcmp(kolor, K6) == 0) return 6;
    else return 7;
}

// Funkcja wypisująca nazwę koloru o danym indeksie
//
//      
void nazwaKoloru(int indeks) {
    if (indeks == 0)
        cout << "green";
    else if (indeks == 1)
        cout << "blue";
    else if (indeks == 2)
        cout << "red";
    else if (indeks == 3)
        cout << "violet";
    else if (indeks == 4)
        cout << "yellow";
    else if (indeks == 5)
        cout << "white";
    else if (indeks == 6)
        cout << "black";
}

// Funkcja używana przy zapisywaniu stanu gry, służy do zapisania nazwy koloru do pliku
//
//      
void nazwaKoloru(int indeks, ofstream& zapis) {

    if (indeks == 0)
        zapis << "green";
    else if (indeks == 1)
        zapis << "blue";
    else if (indeks == 2)
        zapis << "red";
    else if (indeks == 3)
        zapis << "violet";
    else if (indeks == 4)
        zapis << "yellow";
    else if (indeks == 5)
        zapis << "white";
    else if (indeks == 6)
        zapis << "black";
}

// Funkcja używana przy ustalaniu wyniku, służy do podliczenia wyników
//
//      
void podliczWyniki(Dane& dane, WynikiGracza* wynik) {

    for (int i = 0; i < dane.liczbaGraczy; i++) {
        for (int j = 0; j < (dane.liczbaKociolkow + 1); j++) {
            if (j == 0)
                (wynik + i)->wynik = (wynik + i)->wynik + ((wynik + i)->liczbaKart[j] * 2);
            else
                (wynik + i)->wynik = (wynik + i)->wynik + (wynik + i)->liczbaKart[j];
            for (int k = 0; k < (wynik + i)->liczbaOdpornosci; k++) {
                if (j == (wynik + i)->odpornosc[k]) {
                    (wynik + i)->wynik = (wynik + i)->wynik - (wynik + i)->liczbaKart[j];
                    break;
                }
            }
        }

    }
}

// Funkcja używana przy ustalaniu wyniku gry, służy do ustalenia kolorow na które odporni
// są gracze
//
//      Zwraca: indeks koloru
void ustalOdpornosc(WynikiGracza* wynik, Dane& dane) {
    int suma = 0;
    int licznik = 0;
    int indeksOdpornego = (-1);
    int liczbaKolorow = (dane.liczbaKociolkow) + 1;
    for (int i = 1; i < (liczbaKolorow); i++) {

        for (int j = 0; j < dane.liczbaGraczy; j++) {
            if (suma <= (wynik + j)->liczbaKart[i] && (wynik + j)->liczbaKart[i] != 0) {
                if (suma == (wynik + j)->liczbaKart[i]) {
                    licznik = licznik + 1;
                }
                else {
                    suma = (wynik + j)->liczbaKart[i];
                    licznik = 1;
                    indeksOdpornego = j;
                }
            }
        }
        if (licznik > 1)
            indeksOdpornego = (-1);
        if (indeksOdpornego != (-1)) {
            (wynik + indeksOdpornego)->liczbaOdpornosci = (wynik + indeksOdpornego)->liczbaOdpornosci + 1;
            if ((wynik + indeksOdpornego)->liczbaOdpornosci > 1)
                (wynik + indeksOdpornego)->odpornosc = (int*)realloc((wynik + indeksOdpornego)->odpornosc, (wynik + indeksOdpornego)->liczbaOdpornosci * (sizeof(int)));
            (wynik + indeksOdpornego)->odpornosc[(wynik + indeksOdpornego)->liczbaOdpornosci - 1] = i;
        }
        suma = 0;

    }
}

// Funkcja dowodząca ustalaniem wyniku, tworzy tablice dynamiczne do zapisu wynikow
// graczy
//      
void ustalWyniki(WynikiGracza*& wynik, Dane& dane, Gracz* gracz, Talia& talia) {
    int indeks;
    wynik = (WynikiGracza*)malloc(dane.liczbaGraczy * (sizeof(WynikiGracza)));
    for (int i = 0; i < dane.liczbaGraczy; i++) {
        // wynik + liczba - liczba to numer gracza;
        // liczba karty - indeks w liczbie kart odpowiada indeksowi kolorow
        // kolorow jest tyle co liczba kociolkow + 1
        (wynik + i)->liczbaKart = (int*)malloc((dane.liczbaKociolkow + 1) * sizeof(int));
        for (int j = 0; j < (dane.liczbaKociolkow + 1); j++)
            (wynik + i)->liczbaKart[j] = 0; // zeruje u gracza liczbe kart kolorow 
        (wynik + i)->liczbaOdpornosci = 0; // gracz poczatkowo ma 0 odpornosci na kolor
        // gracz moze miec odpornosc na wiele kolorow, oprocz na zielone
        (wynik + i)->wynik = 0;
        (wynik + i)->odpornosc = (int*)malloc(1 * sizeof(int));

        // w odpornosc ma byc zapisany indeks karty, na ktora jest odporny gracz
        for (int j = 0; j < (gracz + i)->liczbaZakrytychKart; j++) {
            // kolory jakie ma gracz bierze sie z kart z zakrytej talii
            // w zakrytej talii jest tylko indeks karty, a kolor jej mozna odczytac 
            // spod indeksu = indeks Karty w talii. kolor[indeks]
            indeks = talia.kolor[(gracz + i)->zakryteKarty[j]];
            (wynik + i)->liczbaKart[indeks] = (wynik + i)->liczbaKart[indeks] + 1;
            // zwiekszanie liczby kart danego koloru 
        }
    }
    ustalOdpornosc(wynik, dane);
    podliczWyniki(dane, wynik);

}

// Funkcja używana przy wypisywaniu wyniku
//
//      Zwraca: indeks gracza lub (-1) jeśli żaden gracz nie jest odporny na dany kolor
int indeksOdpornegoGracza(WynikiGracza* wynik, Dane& dane, int indeksKoloru) {

    for (int i = 0; i < dane.liczbaGraczy; i++) {
        for (int j = 0; j < (wynik + i)->liczbaOdpornosci; j++) {
            if ((wynik + i)->odpornosc[j] == indeksKoloru) {
                return i;
            }
        }
    }
    return -1;
}

// Funkcja służy do wyświetlenia wyników
//
//      
void wyswietlWyniki(Dane& dane, WynikiGracza* wynik) {
    //wyswietlenie odpornosci
    int indeksGracza;
    for (int i = 1; i <= dane.liczbaKociolkow; i++) {
        indeksGracza = indeksOdpornegoGracza(wynik, dane, i);
        if (indeksGracza >= 0) {
            cout << "Na kolor ";
            nazwaKoloru(i);
            cout << " odporny jest gracz " << (indeksGracza + 1) << endl;
        }
    }
    for (int i = 0; i < dane.liczbaGraczy; i++) {
        cout << "Wynik gracza " << (i + 1) << " = " << (wynik + i)->wynik << endl;
    }
}

// Funkcja używana przy sprawdzeniu poprawności gry
//
//      Zwraca: 0 jeśli karty na rękach graczy są poprawne lub 1, jeśli niepoprawne
int sprawdzenieRakGracza(Dane& dane, Gracz* gracz) {
    int liczba = gracz[0].liczbaOdkrytychKart;
    int liczba2;
    for (int i = 0; i < dane.liczbaGraczy; i++) {
        liczba2 = gracz[i].liczbaOdkrytychKart;
        if ((liczba - liczba2) != 0 && (liczba - liczba2) != 1 && (liczba - liczba2) != (-1)) {
            cout << "The number of players cards on hand is wrong" << endl;
            return 1;
        }
    }

    return 0;
}

// Funkcja używana przy sprawdzeniu poprawności gry
//
//      Zwraca: 0 jeśli kolory kart w kociolkach poprawne lub 1, jeśli niepoprawne
int sprawdzenieKolorowWKociolku(Dane& dane, Kociolek* kociolek, Talia& talia) {
    int zielony = 0;
    int kolor = (-1);
    int wynik = 0;
    for (int i = 0; i < dane.liczbaKociolkow; i++) {
        for (int j = 0; j < kociolek[i].liczbaKart; j++) {

            if (talia.kolor[kociolek[i].karty[j]] != zielony) {
                if (kolor != (-1) && kolor != (talia.kolor[kociolek[i].karty[j]])) {
                    cout << "Two different colors were found on the " << (i + 1) << " pile" << endl;
                    wynik = 1;
                    break;
                }
                else
                    kolor = (talia.kolor[kociolek[i].karty[j]]);
            }
        }
        kolor = (-1);
    }
    return wynik;
}

// Funkcja używana przy ustaleniu poprawności wczytywanej gry. Służy ustaleniu 
// poprawności wartości zielonych kart
//
// Zwraca: 0 jeśli jest nie ma zielonych kart; 1 jeśli jest jedna wartość zielonych kart
//                 2 jeśli jest kilka wartości zielonych kart
int znajdzZielone(Talia& talia, Dane& dane) {

    int dobrze = 1;
    int brak;
    int kilka = 0;

    for (int i = 0; i < dane.liczbaWszystkichKart; i++) {
        if (talia.kolor[i] == 0) {
            dane.liczbaZielonych = dane.liczbaZielonych + 1;
            if (dane.wartoscZielonych != (-1) && dane.wartoscZielonych != talia.wartosc[i])
                kilka = 2;
            dane.wartoscZielonych = talia.wartosc[i];
        }
    }
    if (dane.liczbaZielonych == 0) {
        brak = 0;
        return brak;
    }
    else if (kilka == 2) {
        return kilka;
    }
    else {
        return dobrze;
    }
}

// Funkcja używana przy sprawdzeniu poprawności gry
//
//      Zwraca: 0 jeśli kociolek nie powinien eksplodowac wczesniej lub 1, jeśli powinien
int sprawdzenieEkspozji(Dane& dane, Kociolek* kociolek, Talia& talia) {

    int suma = 0;
    int wynik = 0;
    for (int i = 0; i < dane.liczbaKociolkow; i++) {
        for (int j = 0; j < kociolek[i].liczbaKart; j++) {

            suma = suma + talia.wartosc[kociolek[i].karty[j]];
        }
        if (suma >= dane.ekspozjaKociolka) {
            cout << "Pile number " << (i + 1) << " should explode earlier" << endl;
            wynik = 1;

        }
        suma = 0;
    }
    return wynik;
}

// Funkcja używana przy tworzeniu talii potasowanej. 
// zapisuje pod indeksy (które odpowiadaja indeksom kolorow) zmiennej typu Karty
// po kolei wartości z talii nieuporządkowanej
//
void znajdzKolor(Talia& talia, Dane& dane, Karty* karty, int liczbaKolorow) {

    for (int i = 0; i < dane.liczbaWszystkichKart; i++) {
        for (int j = 0; j < liczbaKolorow; j++) {
            if (talia.kolor[i] == j) {
                karty[j].liczbaKart = karty[j].liczbaKart + 1;
                karty[j].wartosc = (int*)realloc(karty[j].wartosc, karty[j].liczbaKart * (sizeof(int)));
                karty[j].wartosc[karty[j].liczbaKart - 1] = talia.wartosc[i];
                break;
            }
        }
    }
}

// Funkcja potrzebna do sortowania wartości, aby otrzymać talię
// potasowaną
//
int my_compare(const void* a, const void* b) {
    int _a = *(int*)a;
    int _b = *(int*)b;
    if (_a < _b) return -1;
    else if (_a == _b) return 0;
    else return 1;
}

// Funkcja tworząca potasowane talie kart kolorów
//
void stworzTaliePosortowane(Karty*& karty, Dane& dane, Talia& talia) {

    karty = (Karty*)calloc((1 + dane.liczbaKociolkow), (sizeof(Karty)));
    for (int i = 0; i <= dane.liczbaKociolkow; i++) {
        karty[i].wartosc = (int*)calloc(1, (sizeof(int)));

        karty[i].kolor = i;
    }
    znajdzKolor(talia, dane, karty, (dane.liczbaKociolkow + 1)); //przyporzadkowywanie kolorow

    for (int i = 0; i <= (dane.liczbaKociolkow); i++) { //sortowanie kart kolorow
        qsort(karty[i].wartosc, karty[i].liczbaKart, sizeof(int), my_compare);

    }
}

// Funkcja używana przy wczytywaniu gry.
// Wyświetla komunikat zależny od liczby różnych wartości zielonych kart
// oraz liczby różnych wartości kart kolorów
// 
void wypiszParametryGry(Karty* karty, Dane& dane, Talia& talia) {
    int wypisz;
    wypisz = znajdzZielone(talia, dane);
    if (wypisz == 1)
        cout << "Found " << dane.liczbaZielonych << " green cards, all with " << dane.wartoscZielonych << " value";
    else if (wypisz == 0)
        cout << "Green cards does not exist";
    else
        cout << "Different green cards values occurred";
    cout << endl;
    karty = (Karty*)calloc((1 + dane.liczbaKociolkow), (sizeof(Karty)));
    for (int i = 0; i <= dane.liczbaKociolkow; i++) {
        karty[i].wartosc = (int*)calloc(1, (sizeof(int)));

        karty[i].kolor = i;
    }
    //wypisanie

    znajdzKolor(talia, dane, karty, (dane.liczbaKociolkow + 1)); //przyporzadkowywanie kolorow

    for (int i = 0; i <= (dane.liczbaKociolkow); i++) { //sortowanie
        qsort(karty[i].wartosc, karty[i].liczbaKart, sizeof(int), my_compare);
    }
    int liczba = 0;
    for (int i = 1; i < dane.liczbaKociolkow + 1; i++) {
        if (liczba != 0 && liczba != karty[i].liczbaKart) {
            //rozne wartosci
            liczba = (-1);
        }
        else {
            liczba = karty[i].liczbaKart;
        }
    }
    if (liczba != (-1)) {
        cout << "The values of cards of all colors are identical: ";
        for (int i = 0; i < karty[1].liczbaKart; i++) {
            cout << karty[1].wartosc[i] << " ";
        }
    }
    else {
        cout << "The values of cards of all colors are not identical:" << endl;
        for (int i = 1; i <= dane.liczbaKociolkow; i++) {
            nazwaKoloru(karty[i].kolor);
            cout << " cards values: ";
            for (int j = 0; j < karty[i].liczbaKart; j++) {
                cout << karty[i].wartosc[j] << " ";
            }
            cout << endl;
        }
    }
}

// Funkcja używana przy wpisywaniu karty do talii, aby określić wartość karty.
//
//      Zwraca: wartość karty
int wartoscKarty(char& ch, FILE* plik) {
    int liczba1, liczba2;
    liczba1 = atoi(&ch); // pobrany znak przed wejsciem do funkcji, to cyfra
    ch = fgetc(plik);
    if (ch == ' ') {
        return liczba1;
    }
    else {
        liczba2 = atoi(&ch);
        ch = fgetc(plik);
        liczba1 = liczba1 * 10 + liczba2;
        return liczba1;
    }
}

// Funkcja służy dopisaniu wartości karty i koloru do talii
// używana przy wczytywaniu stanu gry
//      
void dopiszKarteDoTalii(char& ch, Talia& talia, Dane& dane, FILE* plik) {
    int liczba;
    char kolor[MAKS_LICZBA_KOLOR];
    int indeks = 0;

    liczba = wartoscKarty(ch, plik); // pobrany znak ch przed wejsciem do funkcji, to cyfra

    dane.liczbaWszystkichKart = dane.liczbaWszystkichKart + 1; // bedziemy dopisywac jedna karte
    // realokacja tablic, czyli zwiekszenie ich rozmiaru o jeden
    talia.wartosc = (int*)realloc(talia.wartosc, dane.liczbaWszystkichKart * (sizeof(int)));
    talia.kolor = (int*)realloc(talia.kolor, dane.liczbaWszystkichKart * (sizeof(int)));
    talia.wartosc[dane.liczbaWszystkichKart - 1] = liczba; // dopisuje wartosc karty do talii
    ch = fgetc(plik);

    while (ch != '\n' && ch != ' ' && ch != EOF) { //pobiera znaki koloru do momentu az nie bedzie spacja lub enter 
        kolor[indeks] = ch;
        indeks = indeks + 1;
        ch = fgetc(plik);
    }
    while (indeks < 6) {
        kolor[indeks] = ' ';
        indeks = indeks + 1;
    }

    kolor[indeks] = '\0';
    talia.kolor[dane.liczbaWszystkichKart - 1] = zwrotIndeksuKoloru(kolor);
}

// Funkcja używana przy wkładaniu karty do kociołka. Przyporządkowauje indeks koloru
// do kociołka o danym indeksie
//
void przyporzadkujKolorDoKociolka(Kociolek* kociolek, Dane& dane, Talia& talia, int indeks) {
    int zielony = 0;
    int kolor = (-1);

    for (int j = 0; j < kociolek[indeks].liczbaKart; j++) {

        if (talia.kolor[kociolek[indeks].karty[j]] != zielony) {
            kolor = talia.kolor[kociolek[indeks].karty[j]];
            break;
        }
    }
    kociolek[indeks].kolor = kolor;
}

//Funkcja obsługuje eksplozje kociołka
//
void eksplozja(int indeksKociolka, Dane& dane, Talia& talia, Kociolek* kociolek, Gracz* gracz, int indeksGracza) {

    for (int i = 0; i < (kociolek + indeksKociolka)->liczbaKart; i++) {
        (gracz + indeksGracza)->liczbaZakrytychKart = (gracz + indeksGracza)->liczbaZakrytychKart + 1;
        (gracz + indeksGracza)->zakryteKarty = (int*)realloc((gracz + indeksGracza)->zakryteKarty, (gracz + indeksGracza)->liczbaZakrytychKart * (sizeof(int)));
        (gracz + indeksGracza)->zakryteKarty[(gracz + indeksGracza)->liczbaZakrytychKart - 1] = (kociolek + indeksKociolka)->karty[i];
    }
    (kociolek + indeksKociolka)->liczbaKart = 0;
    (kociolek + indeksKociolka)->karty = (int*)realloc((kociolek + indeksKociolka)->karty, sizeof(int));
    (kociolek + indeksKociolka)->kolor = (-1);
}

// Funkcja sprawdza, czy po posunięciu gracza powinna wystąpić eksplozja
//
void sprawdzenieEkspozji(Dane& dane, Kociolek* kociolek, Talia& talia, int indeks, int indeksGracza, Gracz* gracz) {

    int suma = 0;


    for (int j = 0; j < kociolek[indeks].liczbaKart; j++) {

        suma = suma + talia.wartosc[kociolek[indeks].karty[j]];
    }
    if (suma >= dane.ekspozjaKociolka) {
        eksplozja(indeks, dane, talia, kociolek, gracz, indeksGracza);
    }
}

// Funkcja używana przy posunięciu gracza, szuka kociołka, w który można wstawić kartę
// o danym kolorze
//
// Zwraca: indeks kociołka
int znajdzKociolek(int kolorKarty, Dane& dane, Gracz* gracz, Kociolek* kociolek, Talia& talia) {

    int zielony = 0;
    for (int i = 0; i < dane.liczbaKociolkow; i++) {
        przyporzadkujKolorDoKociolka(kociolek, dane, talia, i);
        if (kolorKarty == zielony || kolorKarty == (kociolek + i)->kolor) {
            return i;
        }
    }
    for (int i = 0; i < dane.liczbaKociolkow; i++) {
        if ((kociolek + i)->kolor == (-1)) {
            return i;
        }
    }
    return (-1);
}

//Funkcja obsługująca posunięcie aktywnego gracza
//
void posuniecie(Dane& dane, Gracz* gracz, Kociolek* kociolek, Talia& talia) {
    int indeksGracza = (dane.aktywnyGracz - 1);
    int indeksKarty = (gracz + indeksGracza)->taliaKarty[0];
    int wartoscKarty = talia.wartosc[indeksKarty];
    int kolorKarty = talia.kolor[indeksKarty];
    int indeksKociolka = znajdzKociolek(kolorKarty, dane, gracz, kociolek, talia);

    //wstawienie do kociolka
    if ((kociolek + indeksKociolka)->liczbaKart == 0) {
        (kociolek + indeksKociolka)->karty = (int*)malloc((kociolek + indeksKociolka)->liczbaKart * (sizeof(int)));
    }
    (kociolek + indeksKociolka)->liczbaKart = (kociolek + indeksKociolka)->liczbaKart + 1;
    (kociolek + indeksKociolka)->karty = (int*)realloc((kociolek + indeksKociolka)->karty, (kociolek + indeksKociolka)->liczbaKart * (sizeof(int)));
    (kociolek + indeksKociolka)->karty[(kociolek + indeksKociolka)->liczbaKart - 1] = indeksKarty;
    //usuniecie z talii gracza
    for (int i = 1; i < (gracz + indeksGracza)->liczbaOdkrytychKart; i++) {
        (gracz + indeksGracza)->taliaKarty[i - 1] = (gracz + indeksGracza)->taliaKarty[i];
    }
    (gracz + indeksGracza)->liczbaOdkrytychKart = (gracz + indeksGracza)->liczbaOdkrytychKart - 1;

    sprawdzenieEkspozji(dane, kociolek, talia, indeksKociolka, indeksGracza, gracz);
    dane.aktywnyGracz = dane.aktywnyGracz + 1;
    if (dane.aktywnyGracz == (dane.liczbaGraczy + 1)) {
        dane.aktywnyGracz = 1;
    }
}

// Funkcja używana przy zapisywaniu gry do pliku
// wpisuje karty odkryte i zakryte graczy
//
void wypiszKartyGraczy(Dane& dane, Gracz* gracz, Talia& talia, ofstream& plik) {
    int wartoscKarty;
    int kolorKarty;
    for (int i = 0; i < dane.liczbaGraczy; i++) {

        plik << (i + 1) << " player hand cards: ";
        for (int j = 0; j < (gracz + i)->liczbaOdkrytychKart; j++) {
            wartoscKarty = talia.wartosc[(gracz + i)->taliaKarty[j]];
            kolorKarty = talia.kolor[(gracz + i)->taliaKarty[j]];
            plik << wartoscKarty << " ";
            nazwaKoloru(kolorKarty, plik);
            plik << " ";
        }
        plik << endl;
        plik << (i + 1) << " player deck cards: ";
        for (int j = 0; j < (gracz + i)->liczbaZakrytychKart; j++) {
            wartoscKarty = talia.wartosc[(gracz + i)->zakryteKarty[j]];
            kolorKarty = talia.kolor[(gracz + i)->zakryteKarty[j]];
            plik << wartoscKarty << " ";
            nazwaKoloru(kolorKarty, plik);
            plik << " ";
        }
        plik << endl;
    }
}

// Funkcja używana przy zapisywaniu gry do pliku
// wpisuje karty w kociolkach
//
void wypiszKartyWKociolku(Dane& dane, Kociolek* kociolek, Talia& talia, ofstream& plik) {
    int wartoscKarty;
    int kolorKarty;

    for (int i = 0; i < dane.liczbaKociolkow; i++) {
        plik << (i + 1) << " pile cards: ";
        for (int j = 0; j < (kociolek + i)->liczbaKart; j++) {
            wartoscKarty = talia.wartosc[(kociolek + i)->karty[j]];
            kolorKarty = talia.kolor[(kociolek + i)->karty[j]];
            plik << wartoscKarty << " ";
            nazwaKoloru(kolorKarty, plik);
            plik << " ";
        }
        if (i != dane.liczbaKociolkow - 1)
            plik << endl;
    }
}

// Funkcja obsługująca zapis gry do pliku
// zapisuje dane o grze, a później przekierowuje do dalszych funkcji
//
void wypiszStanGry(Dane& dane, Gracz* gracz, Kociolek* kociolek, Talia& talia, ofstream& plik) {
    plik << "active player = " << dane.aktywnyGracz << endl;
    plik << "players number = " << dane.liczbaGraczy << endl;
    plik << "explosion threshold = " << dane.ekspozjaKociolka << endl;

    wypiszKartyGraczy(dane, gracz, talia, plik);
    wypiszKartyWKociolku(dane, kociolek, talia, plik);
}

// Funkcja używana przy wczytywaniu stanu gry.
// Wczytuje talie graczy odkryte i zakryte
//
void wczytajTalieGracza(char& znak, Dane& dane, Talia& talia, Gracz* gracz, int& warunek, int& numerLinijki, FILE* plik) {

    int numerGracza;
    int liczba;
    if (znak == '\n' || znak == '1') {

        if (znak == '\n')
            znak = fgetc(plik);
        numerGracza = wartoscKarty(znak, plik) - 1; // pobrany znak ch przed wejsciem do funkcji, to cyfra


    }
    while (warunek == 0) { // ta petla sie robi dopoki jakis warunek jej nie przerwie, bo musza sie najpierw pobrac literki, a potem dopiero
      // cyfry i kolory, ktore maja dla nas znaczenie, a jakby nie bylo tego warunku, to "i" by sie zwiekszalo
      // bez opamietania, bo by nie weszlo w te ify nizej tylko zwiekszylo "i"

        znak = fgetc(plik);

        if (znak > '0' && znak <= '9') // sprawdzenie, czy jest to cyfra, bo moze byc tez literka z poczatku, czego nie chcemy dopisac
        {
            if (numerLinijki % 2 == 0) {
                dopiszKarteDoTalii(znak, talia, dane, plik); // wywolanie funkcji  dopisania
                (gracz + numerGracza)->liczbaOdkrytychKart = (gracz + numerGracza)->liczbaOdkrytychKart + 1;
                (gracz + numerGracza)->taliaKarty = (int*)realloc((gracz + numerGracza)->taliaKarty, (gracz + numerGracza)->liczbaOdkrytychKart * (sizeof(int)));
                (gracz + numerGracza)->taliaKarty[(gracz + numerGracza)->liczbaOdkrytychKart - 1] = dane.liczbaWszystkichKart - 1;
            }
            else {

                dopiszKarteDoTalii(znak, talia, dane, plik); // wywolanie funkcji 
                (gracz + numerGracza)->liczbaZakrytychKart = (gracz + numerGracza)->liczbaZakrytychKart + 1;
                (gracz + numerGracza)->zakryteKarty = (int*)realloc((gracz + numerGracza)->zakryteKarty, (gracz + numerGracza)->liczbaZakrytychKart * (sizeof(int)));
                (gracz + numerGracza)->zakryteKarty[(gracz + numerGracza)->liczbaZakrytychKart - 1] = dane.liczbaWszystkichKart - 1;
            }

        }

        if (znak == '\n' || znak == EOF) // jesli byl enter, to trzeba warunek na 1 zmienic, zeby ten while sie zakonczyl
            warunek = 1;
    }
    warunek = 0;
}

// Funkcja używana przy wczytywaniu stanu gry.
// Wczytuje karty do kociołków
//
void wczytajKartyDoKociolkow(char& znak, int& warunek, Dane& dane, Kociolek*& kociolek, Talia& talia, FILE* plik) {

    int numerKociolka;
    while (warunek == 0) {
        if (znak == '\n') {

            znak = fgetc(plik);

            numerKociolka = (atoi(&znak) - 1);
            dane.liczbaKociolkow = dane.liczbaKociolkow + 1;
            if (dane.liczbaKociolkow == 1) {
                kociolek = (Kociolek*)malloc(dane.liczbaKociolkow * (sizeof(Kociolek)));
            }
            kociolek = (Kociolek*)realloc(kociolek, dane.liczbaKociolkow * (sizeof(Kociolek)));
            (kociolek + numerKociolka)->liczbaKart = 0;

        }
        znak = fgetc(plik);
        if (znak > '0' && znak <= '9') // sprawdzenie, czy jest to cyfra, bo moze byc tez literka z poczatku, czego nie chcemy dopisac
        {
            dopiszKarteDoTalii(znak, talia, dane, plik); // wywolanie funkcji 
            (kociolek + numerKociolka)->liczbaKart = (kociolek + numerKociolka)->liczbaKart + 1;
            if ((kociolek + numerKociolka)->liczbaKart == 1) {
                (kociolek + numerKociolka)->karty = (int*)malloc((kociolek + numerKociolka)->liczbaKart * (sizeof(int)));
            }
            (kociolek + numerKociolka)->karty = (int*)realloc((kociolek + numerKociolka)->karty, (kociolek + numerKociolka)->liczbaKart * (sizeof(int)));
            (kociolek + numerKociolka)->karty[(kociolek + numerKociolka)->liczbaKart - 1] = dane.liczbaWszystkichKart - 1;

        }
        if (znak == '\n' || znak == EOF) // jesli byl enter, to trzeba warunek na 1 zmienic, zeby ten while sie zakonczyl
            warunek = 1;
    }
    warunek = 0;
}
// Funkcja zarządzająca wczytywaniem gry
// Wczytuje dane o grze, a później przekierowuje do innych funkcji wczytujących,
// aby wczytać talie graczy i karty w kociołku
//
void wczytajStanGry(Dane& dane, Gracz*& gracz, Talia& talia, Kociolek*& kociolek, FILE* plik) {

    int warunek = 0;
    char znak;
    //cin.get(ch);
    fscanf_s(plik, "active player = %d\n", &dane.aktywnyGracz);
    fscanf_s(plik, "players number = %d\n", &dane.liczbaGraczy); //pobieranie liczby graczy
    fscanf_s(plik, "explosion threshold = %d", &dane.ekspozjaKociolka);
    znak = fgetc(plik);
    gracz = (Gracz*)calloc(dane.liczbaGraczy, (sizeof(Gracz)));

    for (int i = 0; i < 2 * dane.liczbaGraczy; i++) { // linijek do pobrania jest tyle ile 2 * liczba graczy
        wczytajTalieGracza(znak, dane, talia, gracz, warunek, i, plik);
    }
    while (znak != EOF) {
        wczytajKartyDoKociolkow(znak, warunek, dane, kociolek, talia, plik);
    }
    for (int i = 0; i < dane.liczbaKociolkow; i++)
        przyporzadkujKolorDoKociolka(kociolek, dane, talia, i);
}

// Funkcja używana przy tworzeniu nowej gry.
// zapisuje parametry gry
// tworzy talię posortowaną
// zapisuje talię do pliku
void stworzGre(Dane& dane, Talia& talia, Gracz*& gracz, Kociolek*& kociolek) {
    cin >> dane.liczbaGraczy;
    cin >> dane.liczbaKociolkow;
    cin >> dane.liczbaZielonych;
    cin >> dane.wartoscZielonych;
    cin >> dane.liczbaKart;

    int* wartoscKarty = (int*)malloc(dane.liczbaKart * (sizeof(int)));
    for (int i = 0; i < dane.liczbaKart; i++) {
        cin >> wartoscKarty[i];
    }
    dane.liczbaWszystkichKart = dane.liczbaZielonych + dane.liczbaKociolkow * dane.liczbaKart;
    talia.wartosc = (int*)malloc(dane.liczbaWszystkichKart * (sizeof(int)));
    talia.kolor = (int*)malloc(dane.liczbaWszystkichKart * (sizeof(int)));
    gracz = (Gracz*)calloc(dane.liczbaGraczy, (sizeof(Gracz)));

    for (int i = 0; i < dane.liczbaZielonych; i++) { // zapisujemy na poczatku talii karty zielone, na indeksach 0....(liczbaZielonych-1)
        talia.wartosc[i] = dane.wartoscZielonych;
        talia.kolor[i] = 0;
    }
    int indeksKarta = 0;
    int numerKoloru = 1;

    for (int i = dane.liczbaZielonych; i < dane.liczbaWszystkichKart; i++) { // zapisujemy karty na pozostalych indeksach
        if (indeksKarta == (dane.liczbaKart)) {
            indeksKarta = 0;
            numerKoloru = numerKoloru + 1;
        }

        talia.wartosc[i] = wartoscKarty[indeksKarta];
        talia.kolor[i] = numerKoloru;
        indeksKarta = indeksKarta + 1;
    }
    dane.aktywnyGracz = 1;
    dane.indeksPierwszegoGrajacego = dane.aktywnyGracz - 1;
    kociolek = (Kociolek*)calloc(dane.liczbaKociolkow, (sizeof(Kociolek)));
    dane.ekspozjaKociolka = EKSPLOZJA;
    ofstream zapis("talia.txt");

    for (int i = 0; i < dane.liczbaWszystkichKart; i++) {
        zapis << talia.wartosc[i] << " ";
        nazwaKoloru(talia.kolor[i], zapis);
        zapis << " ";
    }
    zapis.close();
}

void potasujKarty(Dane& dane, Talia& talia) {
    int rozmiar = dane.liczbaWszystkichKart;
    struct Talia tempTalia;
    tempTalia.wartosc = (int*)malloc(rozmiar * (sizeof(int)));
    tempTalia.kolor = (int*)malloc(rozmiar * (sizeof(int)));
    int* temp;
    srand(time(NULL));
    temp = (int*)malloc(rozmiar * (sizeof(int)));
    for (int i = 0; i < rozmiar; i++) {
        temp[i] = rand() % dane.liczbaWszystkichKart;
        for (int j = 0; j < i; j++) {
            if (temp[i] == temp[j]) {
                temp[i] = rand() % dane.liczbaWszystkichKart;
                j = (-1);
            }
        }
    }
    for (int i = 0; i < dane.liczbaWszystkichKart; i++) {
        tempTalia.wartosc[i] = talia.wartosc[temp[i]];
        tempTalia.kolor[i] = talia.kolor[temp[i]];
    }
    talia.wartosc = tempTalia.wartosc;
    talia.kolor = tempTalia.kolor;

}

// Funkcja używana przy tworzeniu nowej gry.
// Rozdaje karty kolejnym graczom
//
void rozdajKarty(Dane& dane, Gracz* gracz) {
    int indeksGracza = 0;
    int indeks = 0;

    //ROZDAWANIE KART
    for (int i = 0; i < dane.liczbaWszystkichKart; i++) {

        if (indeksGracza == dane.liczbaGraczy) {
            indeksGracza = 0;
            indeks = indeks + 1;
        }
        gracz[indeksGracza].liczbaOdkrytychKart = gracz[indeksGracza].liczbaOdkrytychKart + 1;
        gracz[indeksGracza].taliaKarty = (int*)realloc(gracz[indeksGracza].taliaKarty, (gracz[indeksGracza].liczbaOdkrytychKart) * (sizeof(int)));
        gracz[indeksGracza].taliaKarty[indeks] = i;
        indeksGracza = indeksGracza + 1;
    }
}

// Funkcja używana by zwolnić pamięć
//
void zwolnijPamiec(Talia& talia, Gracz* gracz, Kociolek* kociolek, Dane& dane, WynikiGracza* wyniki, Karty* karty) {
    free(talia.kolor);
    free(talia.wartosc);
    for (int i = 0; i < dane.liczbaGraczy; i++) {

        if ((gracz + i)->liczbaOdkrytychKart > 0)
            free((gracz + i)->taliaKarty);

        if ((gracz + i)->liczbaZakrytychKart > 0)
            free((gracz + i)->zakryteKarty);
        if ((wyniki + i)->liczbaOdpornosci > 0) {
            free((wyniki + i)->odpornosc);
        }
    }

    free(gracz);
    free(wyniki);



    for (int i = 0; i < dane.liczbaKociolkow; i++) {

        if ((kociolek + i)->liczbaKart > 0)
            free((kociolek + i)->karty);

    }
    free(kociolek);
}

void podMenu(Dane& dane, Talia& talia, Gracz* gracz, WynikiGracza* wynik, Kociolek* kociolek) {

    int opcja = 0;
    while (opcja != 6) {
        system("cls");
        cout << "---------------------" << endl;
        cout << "Gra karciana TRUCIZNA" << endl;
        cout << "---------------------" << endl;
        cout << endl << endl;
        cout << "Wybierz opcje: " << endl;
        cout << "1. Proste posuniecie" << endl;
        cout << "2. Wypisz stan gry" << endl;
        cout << "3. Wyswietl wyniki" << endl;
        cout << "4. Automatyczna gra" << endl;
        cout << "5. Zapisz gre" << endl;
        cout << "6. Powrot do menu" << endl;
        cout << endl;
        cout << "Aktywny gracz: " << dane.aktywnyGracz << endl;

        cin >> opcja;
        if ((gracz + (dane.aktywnyGracz - 1) % dane.liczbaGraczy)->liczbaOdkrytychKart == 0 || (gracz + dane.indeksPierwszegoGrajacego)->liczbaOdkrytychKart == 0) {
            cout << endl << "KONIEC GRY" << endl;

            if (opcja == 1 || opcja == 4) {
                cout << "Gra sie skonczyla. Wybierz inna opcje." << endl;
                cin >> opcja;
            }
        }

        switch (opcja) {
        case 1:
            posuniecie(dane, gracz, kociolek, talia);
            break;

        case 3: {
            ustalWyniki(wynik, dane, gracz, talia);
            wyswietlWyniki(dane, wynik);
            cout << endl << "Nacisnij ENTER aby zakonczyc wyswietlanie wynikow";
            getchar();
            getchar();
        }
              break;

        case 4: {
            while ((gracz + (dane.aktywnyGracz - 1) % dane.liczbaGraczy)->liczbaOdkrytychKart > 0 || (gracz + dane.indeksPierwszegoGrajacego)->liczbaOdkrytychKart > 0)
                posuniecie(dane, gracz, kociolek, talia);
        }
              break;

        case 2: {
            //wypisanie
            for (int i = 0; i < dane.liczbaGraczy; i++) {
                cout << (i + 1) << " player has " << (gracz + i)->liczbaOdkrytychKart << " cards on hand" << endl;
                cout << (i + 1) << " player has " << (gracz + i)->liczbaZakrytychKart << " cards in front of him" << endl;
            }
            for (int i = 0; i < dane.liczbaKociolkow; i++) {
                cout << "there are " << (kociolek + i)->liczbaKart << " cards on " << (i + 1) << " pile" << endl;

            }
            cout << endl << "Nacisnij ENTER aby zakonczyc wyswietlanie stanu gry";
            getchar();
            getchar();
        }
              break;

        case 5: {
            ofstream zapis("stanGry.txt");
            wypiszStanGry(dane, gracz, kociolek, talia, zapis);
            zapis.close();
        }
              break;
        }
    }
}

void menu(Dane& dane, Talia& talia, Gracz* gracz, WynikiGracza* wynik, Kociolek* kociolek, Karty*& karty) {
    int wybor;
    system("cls");
    cout << "---------------------" << endl;
    cout << "Gra karciana TRUCIZNA" << endl;
    cout << "---------------------" << endl;
    cout << endl << endl;
    cout << "MENU" << endl << endl;
    cout << "Wybierz opcje: " << endl;
    cout << "1. Stworz nowa gre." << endl;
    cout << "2. Wczytaj ostatni zapis gry." << endl;
    cout << "3. Zapisz gre." << endl;
    cout << "4. Koniec gry." << endl;
    cin >> wybor;

    switch (wybor) {
    case 1: {
        stworzGre(dane, talia, gracz, kociolek);
        potasujKarty(dane, talia);
        rozdajKarty(dane, gracz);
        podMenu(dane, talia, gracz, wynik, kociolek);
    }
          break;

    case 2: {

        FILE* plik;
        plik = fopen("stanGry.txt", "r");
        wczytajStanGry(dane, gracz, talia, kociolek, plik);
        fclose(plik);
        int poprawnosc1, poprawnosc2, poprawnosc3;
        poprawnosc1 = sprawdzenieRakGracza(dane, gracz);
        poprawnosc2 = sprawdzenieKolorowWKociolku(dane, kociolek, talia);
        poprawnosc3 = sprawdzenieEkspozji(dane, kociolek, talia);

        if (poprawnosc1 == 0 && poprawnosc2 == 0 && poprawnosc3 == 0) {
            cout << "Current state of the game is ok" << endl;
            wypiszParametryGry(karty, dane, talia);
            getchar();
            getchar();
            podMenu(dane, talia, gracz, wynik, kociolek);
        }
        else {

            cout << "Wczytywane parametry gry sa nieprawidlowe." << endl;
            getchar();
            getchar();
        }
    }
          break;

    case 3: {
        ofstream zapis("stanGry.txt");
        wypiszStanGry(dane, gracz, kociolek, talia, zapis);
        zapis.close();
    }
          break;

    case 4: {
        zwolnijPamiec(talia, gracz, kociolek, dane, wynik, karty);
        exit(0);
    }
          break;
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    cin.tie(NULL);
    struct Dane dane = {};

    int wybor;

    for (;;) {
        struct Dane dane = {};
        struct Talia talia = {};
        struct Kociolek* kociolek = {};
        struct Gracz* gracz = {};
        struct WynikiGracza* wynik = {};
        struct Karty* karty;
        menu(dane, talia, gracz, wynik, kociolek, karty);
    }

    return 0;
}