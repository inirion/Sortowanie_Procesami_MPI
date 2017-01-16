#include <mpi.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <vector>
#include <map>
#define END 9
#define N 25983
using namespace std;
void Swap(std::vector<int>& vHeap, std::vector<int>::size_type i, std::vector<int>::size_type j)
{
	if (i == j)
		return;
	int temp;
	temp = vHeap[i];
	vHeap[i] = vHeap[j];
	vHeap[j] = temp;
}

void Sift(std::vector<int>& vHeap, const std::vector<int>::size_type heapSize, const std::vector<int>::size_type siftNode)
{
	std::vector<int>::size_type i, j;
	j = siftNode;
	do
	{
		i = j;
		if (((2 * i + 1) < heapSize) && vHeap[j] < vHeap[2 * i + 1])
			j = 2 * i + 1;
		if (((2 * i + 2) < heapSize) && vHeap[j] < vHeap[2 * i + 2])
			j = 2 * i + 2;

		Swap(vHeap, i, j);
	} while (i != j);
}

void MakeInitialHeap(std::vector<int>& vHeap)
{
	for (int i = vHeap.size() - 1; i >= 0; --i)
	{
		Sift(vHeap, vHeap.size(), i);
	}
}

void HeapSort(std::vector<int>& vHeap)
{//algorytm sortujacy HeapSort
	MakeInitialHeap(vHeap);
	for (std::vector<int>::size_type i = vHeap.size() - 1; i > 0; --i)
	{
		Swap(vHeap, i, 0);
		Sift(vHeap, i, 0);
	}
}
void merge(vector<int> a, int m, vector<int> b, int n, vector<int> &sorted) {//algorytm sortujacy elementy przy dodawaniu do tablicy
	int i, j, k;
	j = k = 0;
	for (i = 0; i < m + n;) {
		if (j < m && k < n) {
			if (a[j] < b[k]) {
				sorted[i] = a[j];
				j++;
			}
			else {
				sorted[i] = b[k];
				k++;
			}
			i++;
		}
		else if (j == m) {
			for (; i < m + n;) {
				sorted[i] = b[k];
				k++;
				i++;
			}
		}
		else {
			for (; i < m + n;) {
				sorted[i] = a[j];
				j++;
				i++;
			}
		}
	}
}
bool arrayComparer(vector<int> &a, int asize, vector<int> &b, int bsize) {//Sprawdzenie czy elementy w tablicy sa takie same
	if (asize != bsize) { return false; }
	for (int i = 0; i < asize; ++i) {
		if (a[i] != b[i]) { return false; }
	}
	return true;
}
void ReciveMessageArray(vector<int> &data, int from, int withTag, MPI_Status *myStatus) {//odczytanie wiadomosci do tablicy
	int arraySize;
	MPI_Probe(from, withTag, MPI_COMM_WORLD, myStatus);
	MPI_Get_count(myStatus, MPI_INT, &arraySize);
	data.resize(arraySize);
	MPI_Recv(&data[0], data.size(), MPI_INT, from, withTag, MPI_COMM_WORLD, myStatus);
}
void ReciveMessageSingle(int *data, int from, int withTag, MPI_Status *myStatus) {//odczytanie pojedynczej wiadomosci
	MPI_Recv(data, 1, MPI_INT, from, withTag, MPI_COMM_WORLD, myStatus);
}
int main() {
	int size, rank, iteracja = 1, flaga, NumOfEl, count = 0, tempSize, koniec = 0, lastProcValue, lastOne, i, j, k;
	MPI_Status status;
	vector<int> data(N);
	vector<int> recvBuff1;
	vector<int> recvBuff2;
	int rc = MPI_Init(NULL, NULL);
	if (rc != MPI_SUCCESS) {
		printf("Error starting MPI program. Terminationg. /n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_size(MPI_COMM_WORLD, &tempSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	srand((unsigned)time(NULL) + rank*size);
	if (iteracja == 1) {//inicjalizacja tablicy i pierwsze rozes³anie danych do procesów 
		if (rank == 0) {
			map<int, vector<int>> choppedArray;
			for (i = 0; i < N; i++) {
				data[i] = rand() % 20 - rand() % 20;
			}
			int div = N / (size - 1);
			int mod = N % (size - 1);
			for (i = 0, k = 0; i < N; i++, k++) {
				if (i % (size - 1) == 0) k = 0;
				choppedArray[k].push_back(data[i]);
			}
			for (i = 0; i < size - 1; i++) {
				MPI_Send(&choppedArray[i][0], choppedArray[i].size(), MPI_INT, i + 1, 0, MPI_COMM_WORLD);
			}
			if (size == 2)
				iteracja = 3;
			else
				iteracja = 2;
		}
		else {//sortowanie otrzymanych danych oraz wys³anie ich dalej 
			ReciveMessageArray(recvBuff1, 0, 0, &status);
			if (recvBuff1.size() != N) {
				HeapSort(recvBuff1);
				MPI_Send(&recvBuff1[0], recvBuff1.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
				iteracja = 2;
			}
			else {
				HeapSort(recvBuff1);
				MPI_Send(&recvBuff1[0], recvBuff1.size(), MPI_INT, 0, END, MPI_COMM_WORLD);
				iteracja = 3;
			}
		}
	}
	cout << iteracja << endl;
	if (iteracja == 2) {
		while (size >= 2) {//dopuki wielkosc bêdzie wieksza od 2 ( w tym wypadku wielkosc 2 to nie ilosc procesow tylko ilosc procesow -1)
			if (rank == 0) {//jezeli jestesmy w roocie
				map<int, vector<int>> recvBuffChunks;
				if (count == 0) {//jezeli jest to nasza pierwsza wizyta w roocie odczytujemy dane troszeczke inaczej
					for (i = 1; i < size; i++) {
						MPI_Probe(i, 0, MPI_COMM_WORLD, &status);
						MPI_Get_count(&status, MPI_INT, &NumOfEl);
						recvBuffChunks[i].resize(NumOfEl);
						MPI_Recv(&recvBuffChunks[i][0], recvBuffChunks[i].size(), MPI_INT, i, 0, MPI_COMM_WORLD, &status);
					}
				}
				else if (count == 1) {//odczytywanie wartosci je¿eli jesteœmy juz w pêtli > niz 1 raz, to odbywa sie do koñca.
					for (i = 1; i <= size; i++) {
						MPI_Probe(i, 0, MPI_COMM_WORLD, &status);
						MPI_Get_count(&status, MPI_INT, &NumOfEl);
						recvBuffChunks[i].resize(NumOfEl);
						MPI_Recv(&recvBuffChunks[i][0], recvBuffChunks[i].size(), MPI_INT, i, 0, MPI_COMM_WORLD, &status);
					}
				}
				lastProcValue = size;//ostatnia ilosc procesorow pracowników.
				if (count == 0) {//jezeli jestesmy tutaj 1 raz to zaokraglamy do gory i ustawiamy ilosc procesorow (pracownikow) o jeden mniejsza.
					if ((size - 1) % 2 != 0) {//w wypadkach gdy zmniejszamy ilosc procesow aby byla ona rowna liczbe robotnikow(wejscie 20 procesow na start ale tak naprawde korzystamy z 19)
						size = size / 2;
						lastOne = 2;//ostatnia liczba procesow by³a nieprzysta
						lastProcValue -= 1;
						count++;
					}
					else if ((size - 1) % 2 == 0) {
						size = size / 2;
						lastOne = 1;
						count++;
					}
				}
				else {//to samo co wyzej ale juz za kazdym razem
					if (size % 2 == 0) {
						size = (size) / 2;
						lastOne = 1;
					}
					else {
						size = (size + 2 - 1) / 2;
						lastOne = 2;
					}
				}
				for (int i = 1; i < tempSize; i++) {//wyslanie do procesow wiadomosci o fladze oraz ilosci procesow jakie beda wykonywa³y sortowanie.
					MPI_Send(&size, 1, MPI_INT, i, lastOne, MPI_COMM_WORLD);
				}
				cout << "Wysylam do  " << size << " Procesow do sortowania" << endl;
				for (i = 1, k = 1; i <= size; i++, k += 2) {//pêtla odpowiadajaca za wysylanie odpowiednich kawa³ków tablicy do procesorów, s¹ one usytuowane w mapie <int , vecor> //int zaczyna siê od 1 bez przyczyny tak sobie za³o¿y³em.
						if (lastOne == 1) {
							if (size == 1) {
								MPI_Send(&recvBuffChunks[1][0], recvBuffChunks[1].size(), MPI_INT, 1, 0, MPI_COMM_WORLD);
								MPI_Send(&recvBuffChunks[2][0], recvBuffChunks[2].size(), MPI_INT, 1, 0, MPI_COMM_WORLD);
							}
							else {
								MPI_Send(&recvBuffChunks[k][0], recvBuffChunks[k].size(), MPI_INT, i, 0, MPI_COMM_WORLD);
								MPI_Send(&recvBuffChunks[k + 1][0], recvBuffChunks[k + 1].size(), MPI_INT, i, 0, MPI_COMM_WORLD);
							}
						}
						else if (lastOne == 2) {
							if (i == size) {
								MPI_Send(&recvBuffChunks[lastProcValue][0], recvBuffChunks[lastProcValue].size(), MPI_INT, size, 0, MPI_COMM_WORLD);
							}
							else if (i < size) {
								MPI_Send(&recvBuffChunks[k][0], recvBuffChunks[k].size(), MPI_INT, i, 0, MPI_COMM_WORLD);
								MPI_Send(&recvBuffChunks[k + 1][0], recvBuffChunks[k + 1].size(), MPI_INT, i, 0, MPI_COMM_WORLD);
							}
						}
				}
			}
			if (rank != 0) {//odbieranie danych
				vector<int> merged;//wektor zawierajacy polaczone tablice
				ReciveMessageSingle(&size, 0, MPI_ANY_TAG, &status);//pobranie flag 
				if (rank <= size && size != 1 && status.MPI_TAG == 1) {
					ReciveMessageArray(recvBuff1, 0, MPI_ANY_TAG, &status);
					ReciveMessageArray(recvBuff2, 0, MPI_ANY_TAG, &status);
					merged.resize(recvBuff1.size() + recvBuff2.size());
					merge(recvBuff1, recvBuff1.size(), recvBuff2, recvBuff2.size(), merged);//laczenie posortowanych tablic w jedna posortowana
					MPI_Send(&merged[0], merged.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
				}
				if (rank <= size && size != 1 && status.MPI_TAG == 2) {
					if (rank < size) {
						ReciveMessageArray(recvBuff1, 0, 0, &status);
						ReciveMessageArray(recvBuff2, 0, 0, &status);
						merged.resize(recvBuff1.size() + recvBuff2.size());
						merge(recvBuff1, recvBuff1.size(), recvBuff2, recvBuff2.size(), merged);//laczenie posortowanych tablic w jedna posortowana
						MPI_Send(&merged[0], merged.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
					}
					else if (rank == size) {// jezeli odebralismy tylko 1 tablice z wartosciami przesylamy ja dalej bo jest posortowana
						ReciveMessageArray(recvBuff1, 0, 0, &status);
						MPI_Send(&recvBuff1[0], recvBuff1.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
					}
				}
				if (size == 1 && rank == 1) {//gdy zosta³y ostanie dwie wartosci w jedym procesie
					ReciveMessageArray(recvBuff1, 0, 0, &status);
					ReciveMessageArray(recvBuff2, 0, 0, &status);
					merged.resize(recvBuff1.size() + recvBuff2.size());
					merge(recvBuff1, recvBuff1.size(), recvBuff2, recvBuff2.size(), merged);//laczenie posortowanych tablic w jedna posortowana
					MPI_Send(&merged[0], merged.size(), MPI_INT, 0, END, MPI_COMM_WORLD);
				}
				else {//wszystkie inne procesy poza tymi wyznaczonymi sa nieczynne(zakoczy³y pracê)
					koniec = 1;
				}
			}
		}
	}

	if (rank == 0) {//odbieranie ostateczej tablicy od procesu 1 
		ReciveMessageArray(recvBuff1, 1, END, &status);
		HeapSort(data);//sortowanie tablicy wygenerowanej
		bool result = arrayComparer(recvBuff1, recvBuff1.size(), data, data.size());//sprawdzenie czy tablice sa równe sobie 
		cout << "Equal ? " << result << endl;// wyswietlenie wyniku
	}
	MPI_Finalize();

	return 0;
}