#include <iostream>
#include <time.h>
#include <fstream>
#include <string>

#define ROZMIAR 1024

float M_PI = atan(1) * 4;
unsigned long long int liczbaOdczytow = 0; // liczba odczytow danych z dysku
unsigned long long int liczbaZapisow = 0; // liczba zapisow danych na dysk
unsigned long long int fazRozdzielania = 0; // ilosc faz rozdzielania tasmy wejsciowej
unsigned long long int fazLaczenia = 0; // ilosc faz laczenia tasm

void wypiszStozek(double r, double h)
{
	printf("r = %f\th = %f\n", r, h);
}

struct Stozek 
{
	double promien; // dlugosc promienia
	double wysokosc; // dlugosc wysokosci
	double objetosc; // wartosc obejtosci (objetosc = 1/3 * PI * r^2 * h)
	
	Stozek() { }

	Stozek(double r, double h)
	{
		promien = r;
		wysokosc = h;
		objetosc = M_PI * r * r * h / 3;
	}

	Stozek(double r, double h, double v)
	{
		promien = r;
		wysokosc = h;
		objetosc = v;
	}
};

struct Plik
{
	char *nazwaPliku; // nazwa pliku
	int miejsceWPliku; // miejsce w pliku, w ktorym zakonczylismy odczytywanie

	Plik (char *nazwa, int miejsce)
	{
		nazwaPliku = nazwa;
		miejsceWPliku = miejsce;
	}

	void wyczyscPlik()
	{
		remove(nazwaPliku);
	}
};

struct BuforGlowny
{
	Stozek *bufor; // wskaznik na tablicê struktur opisuj¹cych rekordy
	int aktualny; // indeks kolejnego rekordu do odczytu
	int rozmiarBufora; // rozmiar bufora
	int licznik; // indeks ostatniego rekordu w buforze
	bool koniecPliku; // czy dotarlismy do konca pliku
	Plik *plikOdczytu; // wskaznik na strukture z informacjami o pliku, z ktorego bedziemy odczytywac dane

	BuforGlowny(Plik *plik)
	{
		rozmiarBufora = ROZMIAR; 
		bufor = new Stozek[rozmiarBufora]; 
		aktualny = rozmiarBufora; 
		koniecPliku = false; 
		licznik = 0; 
		plikOdczytu = plik;
	}

	Stozek* podajRekord()
	{
		if (aktualny == rozmiarBufora)
		{
			liczbaOdczytow++;

			double wartosc1, wartosc2; 
			Stozek *nowy; 

			std::ifstream wejscie(plikOdczytu->nazwaPliku); 
			if (!wejscie.good()) // jezeli wystapil blad podczas otwierania pliku
			{
				std::cout << "Nie mozna otworzyc pliku: " << plikOdczytu->nazwaPliku << std::endl;
				return NULL;
			}

			wejscie.seekg(plikOdczytu->miejsceWPliku,std::ios::beg); 

			licznik = 0;

			while ((licznik < rozmiarBufora) && (!koniecPliku)) 
			{
				if (wejscie >> wartosc1 >> wartosc2)
				{
					nowy = new Stozek(wartosc1, wartosc2);			
					bufor[licznik] = *nowy;
					licznik++;

					wypiszStozek(wartosc1, wartosc2);
				}
				else koniecPliku = true;
			}
			
			plikOdczytu->miejsceWPliku = wejscie.tellg();
			wejscie.close();

			aktualny = 0;
		}

		// jezeli wypisalimy caly bufor i doszlismy do konca pliku to konczymy
		if (aktualny >= licznik && koniecPliku) return NULL;

		aktualny++;
		// zwracamy kolejny element
		return &(bufor[aktualny-1]); // zwaracamy rekord
	}

	Stozek* kolejnyRekord()
	{
		if (aktualny == rozmiarBufora)
		{
			liczbaOdczytow++;

			double wartosc1, wartosc2, wartosc3; 
			Stozek *nowy; 

			std::ifstream wejscie(plikOdczytu->nazwaPliku);
			if (!wejscie.good()) // jezeli wystapil blad podczas otwierania pliku
			{
				std::cout << "Nie mozna otworzyc pliku: " << plikOdczytu->nazwaPliku << std::endl;
				return NULL;
			}
			wejscie.seekg(plikOdczytu->miejsceWPliku,std::ios::beg);

			licznik = 0;

			while ((licznik < rozmiarBufora) && (!koniecPliku)) 
			{
				if (wejscie >> wartosc1 >> wartosc2 >> wartosc3)
				{
					nowy = new Stozek(wartosc1, wartosc2, wartosc3);
				
					bufor[licznik] = *nowy;
					licznik++;
				}
				else
					koniecPliku = true;
			}
			plikOdczytu->miejsceWPliku = wejscie.tellg();
			wejscie.close();

			aktualny = 0;
		}

		if (aktualny >= licznik && koniecPliku) return NULL;

		aktualny++;
		return &(bufor[aktualny-1]);
	}
};

struct BuforTasmy
{
	Stozek *bufor; // wskaznik na bufor
	int aktualny; // indeks kolejnego elementu w buforze do zapisu
	int rozmiarBufora; // rozmiar bufora zapisu
	bool wyswietlic; // czy wyswietlic plik po kolejnych przebiegach (t - tak)
	Plik *plikZapisu; // obiekt opisujacy plik zapisu

	// konstruktory
	BuforTasmy(Plik *plik)
	{
		rozmiarBufora = ROZMIAR;
		bufor = new Stozek[rozmiarBufora];
		aktualny = 0;
		plikZapisu = plik;
		plikZapisu->wyczyscPlik();
		wyswietlic = false;
	}
	BuforTasmy(Plik *plik, char pokaz)
	{
		rozmiarBufora = ROZMIAR;
		bufor = new Stozek[rozmiarBufora];
		aktualny = 0;
		plikZapisu = plik;
		plikZapisu->wyczyscPlik();
		if (pokaz == 't') wyswietlic = true;
		else wyswietlic = false;
	}

	~BuforTasmy ()
	{
		liczbaZapisow++;
		std::ofstream wyjscie(plikZapisu->nazwaPliku, std::ios::out|std::ios::app);
		
		for (int i = 0; i < aktualny; i++)
		{
			wyjscie << bufor[i].promien << " " << bufor[i].wysokosc << " " << bufor[i].objetosc << std::endl;
			if (wyswietlic)
			{
				wypiszStozek(bufor[i].promien, bufor[i].wysokosc);
			}
		}
		wyjscie.close();
	}

	bool zapiszRekord (Stozek *rekord)
	{
		if (aktualny == rozmiarBufora)
		{
			liczbaZapisow++;

			std::ofstream wyjscie(plikZapisu->nazwaPliku, std::ios::out|std::ios::app);

			if (wyswietlic) std::cout << "\n\n\n\nPLIK PO KOLEJNYM PRZEBIEGU:\n\n";

			for (aktualny = 0; aktualny < rozmiarBufora; aktualny++)
			{
				wyjscie << bufor[aktualny].promien << " " << bufor[aktualny].wysokosc << " " << bufor[aktualny].objetosc << std::endl;

				if (wyswietlic) wypiszStozek(bufor[aktualny].promien, bufor[aktualny].wysokosc);
			}
			
			wyjscie.close();
			
			aktualny = 0;
		}
		
		if (rekord == NULL) return false; 
		
		bufor[aktualny] = *rekord;
		
		aktualny++;

		return true;
	}

	bool zapiszWartosci (Stozek *rekord)
	{
		if (aktualny == rozmiarBufora)
		{
			liczbaZapisow++;

			std::ofstream wyjscie(plikZapisu->nazwaPliku, std::ios::out|std::ios::app);

			for (aktualny = 0; aktualny < rozmiarBufora; aktualny++)
			{
				wyjscie << bufor[aktualny].promien << " " << bufor[aktualny].wysokosc << std::endl;
				wypiszStozek(bufor[aktualny].promien, bufor[aktualny].wysokosc);
			}

			wyjscie.close();

			aktualny = 0;
		}

		if (rekord == NULL) return false; 
		bufor[aktualny] = *rekord;

		aktualny++;

		return true;
	}

	void zapiszPozostaleWartosci()
	{
		liczbaZapisow++;

		std::ofstream wyjscie(plikZapisu->nazwaPliku, std::ios::out|std::ios::app);

		for (int i = 0; i < aktualny; i++)
		{
			wyjscie << bufor[i].promien << " " << bufor[i].wysokosc << std::endl;
			wypiszStozek(bufor[i].promien, bufor[i].wysokosc);
		}
		wyjscie.close();

		aktualny = 0;
	}
};

void generujRekordy(char *sciezkaDoPliku)
{
	double promien, wysokosc, czescDziesietna;
	int ile;

	Plik *wyjsciowy = new Plik(sciezkaDoPliku, 0);
	BuforTasmy *tasma = new BuforTasmy(wyjsciowy);
	Stozek *rekord = NULL;

	srand (time(NULL));

	std::cout << "\nPodaj ilosc rekordow do wygenerowania:\n";
	std::cin >> ile;
	while (ile--)
	{
		czescDziesietna = rand()%10000;
		promien = rand()%1000 + czescDziesietna /10000;

		czescDziesietna = rand()%10000;
		wysokosc = rand()%1000 + czescDziesietna /10000;

		rekord = new Stozek(promien, wysokosc);

		tasma->zapiszWartosci(rekord);
	}
	tasma->zapiszPozostaleWartosci();

	delete tasma;
}

void wczytajRekordy(char *sciezkaDoPliku)
{
	double promien, wysokosc;
	int ile;

	Plik *wyjsciowy = new Plik(sciezkaDoPliku, 0);
	BuforTasmy *tasma = new BuforTasmy(wyjsciowy);
	Stozek *rekord = NULL;

	std::cout << "\nPodaj ilosc rekordow do wczytania:\n";
	std::cin >> ile;
	while (ile--)
	{
		std::cout << "Podaj promien:\n";
		std::cin >> promien;
		std::cout << "Podaj wysokosc:\n";
		std::cin >> wysokosc;

		rekord = new Stozek(promien, wysokosc);

		tasma->zapiszWartosci(rekord);
	}
	tasma->zapiszPozostaleWartosci();

	delete tasma;
}

void wczytajNazwePliku(char *sciezkaDoPliku)
{
	int i;
	std::string str;
	std::cout << "Podaj sciezke do pliku:\n";
	std::getline (std::cin,str);
	std::getline (std::cin,str);

	for (i = 0; i < str.length(); i++)
		sciezkaDoPliku[i] = str[i];
	sciezkaDoPliku[i] = '\0';
}

void menuPoczatkowe(char* sciezkaDoPliku)
{
	int instrukcja = -1;

	std::cout << "Wybierz sposob stworzenia pliku z rekordami:\n";
	std::cout << "1. Losowa generacja rekordow\n";
	std::cout << "2. Wpisanie rekordow z klawiatury\n";
	std::cout << "3. Wczytanie rekordow z pliku\n";

	while (instrukcja != 1 && instrukcja != 2 && instrukcja != 3) std::cin >> instrukcja;

	switch (instrukcja)
	{
		case 1: 
			generujRekordy(sciezkaDoPliku); 
			break; 
		case 2: 
			wczytajRekordy(sciezkaDoPliku); 
			break; 
		case 3: 
			wczytajNazwePliku(sciezkaDoPliku); 
			break; 
	}
}

void przepiszPlik (char *plikWejsciowy, char *plikWyjsciowy)
{
	Plik *wejsciowy = new Plik(plikWejsciowy, 0);
	Plik *wyjsciowy = new Plik(plikWyjsciowy, 0);

	BuforGlowny *oryginal = new BuforGlowny(wejsciowy);
	BuforTasmy *kopia = new BuforTasmy(wyjsciowy);

	std::cout << "\nPLIK PRZED POSORTOWANIEM:\n";
	while (kopia->zapiszRekord(oryginal->podajRekord())) ;
	
	delete oryginal;
	delete kopia;
}

bool laczenie(char wyswietlic)
{
	fazLaczenia++;

	Plik* wejsciowyA = new Plik((char*)"a.tape", 0);
	Plik* wejsciowyB = new Plik((char*)"b.tape", 0);
	Plik* wyjsciowy = new Plik((char*)"c.tape", 0);

	BuforGlowny* tasmaA = new BuforGlowny(wejsciowyA);
	BuforGlowny* tasmaB = new BuforGlowny(wejsciowyB);
	BuforTasmy* tasmaC = new BuforTasmy(wyjsciowy, wyswietlic);

	double poprzedniaA = 0, poprzedniaB = 0;
	Stozek* rekordA = tasmaA->kolejnyRekord();
	Stozek* rekordB = tasmaB->kolejnyRekord();

	if (rekordB == NULL) return false;

	while (1)
	{
		if ((rekordA != NULL) && (rekordB != NULL))
		{
			if (rekordA->objetosc < poprzedniaA)
			{
				while ((rekordB != NULL) && (rekordB->objetosc > poprzedniaB))
				{
					tasmaC->zapiszRekord(rekordB);
					poprzedniaB = rekordB->objetosc;
					rekordB = tasmaB->kolejnyRekord();
				}
				poprzedniaA = 0;
				poprzedniaB = 0;
			}
			else if (rekordB->objetosc < poprzedniaB)
			{
				while ((rekordA != NULL) && (rekordA->objetosc > poprzedniaA))
				{
					tasmaC->zapiszRekord(rekordA);
					poprzedniaA = rekordA->objetosc;
					rekordA = tasmaA->kolejnyRekord();
				}
				poprzedniaA = 0;
				poprzedniaB = 0;
			}
			else
			{
				if (rekordA->objetosc < rekordB->objetosc)
				{
					tasmaC->zapiszRekord(rekordA);
					poprzedniaA = rekordA->objetosc;
					rekordA = tasmaA->kolejnyRekord();
				}
				else
				{
					tasmaC->zapiszRekord(rekordB);
					poprzedniaB = rekordB->objetosc;
					rekordB = tasmaB->kolejnyRekord();
				}
			}
		}
		else if (rekordA == NULL)
		{
			while (rekordB != NULL)
			{
				tasmaC->zapiszRekord(rekordB);
				rekordB = tasmaB->kolejnyRekord();
			}
			break;
		}
		else if (rekordB == NULL)
		{
			while (rekordA != NULL)
			{
				tasmaC->zapiszRekord(rekordA);
				rekordA = tasmaA->kolejnyRekord();
			}
			break;
		}
	}
	delete tasmaA;
	delete tasmaB;
	delete tasmaC;

	return true;
}

bool sortuj (char wyswietlic)
{
	fazRozdzielania++;

	Plik *wejsciowy = new Plik((char*)"c.tape", 0);
	Plik *wyjsciowyA = new Plik((char*)"a.tape", 0);
	Plik *wyjsciowyB = new Plik((char*)"b.tape", 0);

	BuforGlowny *tasmaC = new BuforGlowny(wejsciowy);
	BuforTasmy *tasmaA = new BuforTasmy(wyjsciowyA);
	BuforTasmy *tasmaB = new BuforTasmy(wyjsciowyB);

	Stozek *rekord = NULL;
	double poprzedniaWartosc = 0;
	BuforTasmy *tasma = tasmaA;

	while (1)
	{
		rekord = tasmaC->kolejnyRekord();
		if (rekord == NULL) break;

		if (poprzedniaWartosc > rekord->objetosc)
		{
			if (tasma == tasmaA) tasma = tasmaB;
			else tasma = tasmaA;
		}
		tasma->zapiszRekord(rekord);
		poprzedniaWartosc = rekord->objetosc;
	}
	delete tasmaA;
	delete tasmaB;
	delete tasmaC;
	return laczenie(wyswietlic);
}

void przepiszPosortowany()
{
	Plik *wejsciowy = new Plik((char*)"a.tape", 0);
	Plik *wyjsciowy = new Plik((char*)"posortowany.txt", 0);

	BuforGlowny *oryginal = new BuforGlowny(wejsciowy);
	BuforTasmy *kopia = new BuforTasmy(wyjsciowy);

	std::cout << "\nPLIK PO POSORTOWANIU:\n";
	while (kopia->zapiszWartosci(oryginal->kolejnyRekord())) ;
	kopia->zapiszPozostaleWartosci();

	delete oryginal;
	delete kopia;
	remove("a.tape");
	remove("b.tape");
}

int main()
{
	char sciezkaDoPliku[100] = { "stozki.txt" };
	bool czySortowac = true;
	char wyswietlic;

	menuPoczatkowe(sciezkaDoPliku);
	przepiszPlik(sciezkaDoPliku, (char*)"c.tape");
	std::cout << "Czy chcesz wyœwietlic plik po ka¿dej fazie sortowania? (t/n)\n";
	std::cin >> wyswietlic;

	while (czySortowac)
	{
		czySortowac = sortuj(wyswietlic);
	}
	przepiszPosortowany();

	std::cout << "\n\nSTATYSTYKI\n";
	std::cout << "Fazy rozdzielania: " << fazRozdzielania << std::endl;
	std::cout << "Fazy laczenia: " << fazLaczenia << std::endl;
	std::cout << "Odczyty z dysku: " << liczbaOdczytow << std::endl;
	std::cout << "Zapisy na dysk: " << liczbaZapisow << std::endl << std::endl;

	system("pause");
	return 0;
}