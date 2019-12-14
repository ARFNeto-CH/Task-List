#pragma once
#define         _CRT_SECURE_NO_WARNINGS
#define         _ARFNETO_ CH2019

#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>

#include "stdio.h"
#include "stdlib.h"
#include "time.h"


struct snapshot
{
    int              total;
    time_t           hora;
    PROCESSENTRY32** processo;
};
typedef	struct snapshot		Snapshot;


// novas funcoes
Snapshot*       build_snapshot(int);
int				compara_snapshots(Snapshot*, Snapshot*);
int			    get_process_count();
int				insere_processo(PROCESSENTRY32*, Snapshot*);
unsigned int	lista_snapshot(Snapshot*);


int main(int argc, char** argv)
{
    char asw;
    int c = get_process_count(); // quantos processos?
    printf("\nARFNeto 2019 para o Clube do Hardware - uso livre\n");
    printf("\nTotal de %d processos rodando agora\n", c);
    Snapshot* l = build_snapshot(c);

    lista_snapshot(l);
    printf("\nTecle ENTER para ler novo snapshot: ");
    asw = fgetc(stdin);
    printf("\n");

    c = get_process_count(); // quantos processos?
    printf("\nTotal de %d processos rodando agora\n", c);
    Snapshot* m = build_snapshot(c);
    lista_snapshot(m);

    printf("\nTecle ENTER para comparar os dois e sair: ");
    asw = fgetc(stdin);
    printf("\n");
    compara_snapshots(l, m);

    return 0;
};  // main()


//
// cria uma estrutura snapshot e devolve seu endereco
// considerando uma estimativa de processos e alocando
// o dobro de espaco so por seguranca
Snapshot* build_snapshot(int estimativa)
{
    HANDLE          hProcesso;
    HANDLE          hSnap;
    PROCESSENTRY32* p;

    int             total_processos = 0;
    int             sem_acesso = 0;
    int             ss = 0;

    estimativa = (estimativa < 128) ? 128 : estimativa; // assume minimo de 128
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
    {
        printf("CreateToolhelp32Snapshot error\n");
        return(NULL);
    };  // if

    // cria a estrutura com o dobro da estimativa de processos
    Snapshot* lista = (Snapshot*)malloc(sizeof(Snapshot));
    lista->total = 0;
    lista->hora = time(NULL);
    lista->processo =
        (PROCESSENTRY32**)malloc(sizeof(PROCESSENTRY32*) * 2 * estimativa);
    int n = sizeof(PROCESSENTRY32*) * 2 * estimativa;
    // cria um registro de processo
    p = (PROCESSENTRY32*)malloc(sizeof(PROCESSENTRY32));
    p->dwSize = sizeof(PROCESSENTRY32);
    p->th32ProcessID = 3889;

    if (!Process32First(hSnap, p))
    {   // primeiro processo em p
        printf("Erro ao ler primeiro processo\n");
        perror("Process32First()");
        free(p);
        free(lista);
        CloseHandle(hSnap);
        return(NULL);
    };  // if
    // agora salva na lista e le o proximo ate o final
    do
    {   // entra com um processo em p
        total_processos += 1; // pode ser que nao tenhamos acesso
        hProcesso = OpenProcess(PROCESS_ALL_ACCESS, FALSE, p->th32ProcessID);
        if (hProcesso != NULL)
        {   // ok esse deu pra ler             (*(snap->processo + i))->th32ProcessID,
            insere_processo(p, lista);
            CloseHandle(hProcesso);  // libera o handle
            // prepara o proximo
            p = (PROCESSENTRY32*)malloc(sizeof(PROCESSENTRY32));
            p->dwSize = sizeof(PROCESSENTRY32);
        }
        else
        {
            sem_acesso = sem_acesso + 1;
        }
    } while (Process32Next(hSnap, p));
    free(p);    // alocou uma mais no loop
    CloseHandle(hSnap);
    return lista;
};  // build_snapshot()


//
// usa o campo dwFlags para marcar em uma lista o que nao
// esta na outra. Assim temos os processos novos e os encerrados
//
int				compara_snapshots(Snapshot* A, Snapshot* B)
{
    int i, j, n;
    char a_hora[40];
    char* quando = a_hora;
    quando = ctime(&A->hora);
    n = strlen(quando) - 1;
    quando[n] = 0;  // era um \n

    printf("\ncompara_snapshots()\n\n");

    printf("Snapshot A(%s): %d processos\n", quando, A->total);
    quando = ctime(&B->hora);
    n = strlen(quando) - 1;
    quando[n] = 0;  // era um \n
    printf("Snapshot B(%s): %d processos\n", quando, B->total);

    for (i = 0; i < A->total; i += 1)
    {
        DWORD pa = (*(A->processo + i))->th32ProcessID;
        for (j = 0; j < B->total; j += 1)
        {
            DWORD pb = (*(B->processo + j))->th32ProcessID;
            if (pb > pa)
            {   // sumiu pa
                (*(A->processo + i))->dwFlags = 1;
                break;
            };  // if
            if (pb == pa) break; // esse permanece
        };  // for
    }   // for

    // agora ao contrario
    for (i = 0; i < B->total; i += 1)
    {
        DWORD pb = (*(B->processo + i))->th32ProcessID;
        for (j = 0; j < A->total; j += 1)
        {
            DWORD pa = (*(A->processo + j))->th32ProcessID;
            if (pa > pb)
            {   // sumiu pb
                (*(B->processo + i))->dwFlags = 1;
                break;
            };  // if
            if (pb == pa) break; // esse permanece
        };  // for
    }   // for

    // lista os que se foram
    printf("\nProcessos encerrados:\n\n");
    int enc = 0;
    for (i = 0; i < A->total; i += 1)
    {
        if ((*(A->processo + i))->dwFlags == 1)
        {
            enc += 1;
            printf("%03d: pID=%8d exe [%ls]\n",
                enc,
                (*(A->processo + i))->th32ProcessID,
                (*(A->processo + i))->szExeFile
            );
        };  // if
    };  // for

    // lista os que foram iniciados depois
    printf("\nProcessos criados:\n\n");
    int criados = 0;
    for (i = 0; i < B->total; i += 1)
    {
        if ((*(B->processo + i))->dwFlags == 1)
        {
            criados += 1;
            printf("%03d: pID=%8d exe [%ls]\n",
                criados,
                (*(B->processo + i))->th32ProcessID,
                (*(B->processo + i))->szExeFile
            );
        };  // if
    };   // for

    printf("\nIntervalo entre as consultas: %gs\n\n", difftime(B->hora, A->hora));
    return 0;
};  // compara_snapshots()


//
// return the number of processes running, to aid in dimensioning the
// arrays used to compare snapshots
// conta quantos processos estao rodando. Eu ia escrever tudo em ingles
// mas parei :)
int             get_process_count()
{
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    int total_processes = 0;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return(-1);
    }
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);
        return(-2);
    }
    do
        total_processes += 1;
    while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return(total_processes);
};  // get_process_count()


//
// faz o obvio: insere o processo p na estrutura de snapshot lista
int				insere_processo(PROCESSENTRY32* processo, Snapshot* lista)
{
    //printf("insere_processo(%d): %ls na lista\n",
    //    processo->th32ProcessID,
    //    processo->szExeFile
    //);
    //printf("Lista com %03d processos ate aqui\n",
    //    lista->total
    //);

    int pos = lista->total;
    while (pos > 0)
    {
        DWORD atual = (*(lista->processo + pos - 1))->th32ProcessID;
        // printf("processo atual: %d\n", atual);
        if (atual > processo->th32ProcessID)
        {   // abre aqui
            (*(lista->processo + pos)) = (*(lista->processo + pos - 1));
            pos = pos - 1;
        }
        else
        {
            break;
        }
    };  // while
    // vai inserir o novo sempre em pos
    processo->dwFlags = 0; // para usar no controle de processos
    (*(lista->processo + pos)) = processo;
    lista->total += 1;
    return 0;
};  // insere_processo()


// apenas lista dois campos do snapshot
unsigned int    lista_snapshot(Snapshot* snap)
{
    char a_hora[40];
    char* quando = a_hora;
    quando = ctime(&snap->hora);
    int n = strlen(quando) - 1;
    quando[n] = 0; // era um \n
    printf("\nlista_snapshot(em %s):\n\n%d processos [com acesso permitido]\n\n", quando, snap->total);
    for (int i = 0; i < snap->total; i += 1)
    {
        printf("%03d: pID=%8d exe [%ls]\n",
            i,
            (*(snap->processo + i))->th32ProcessID,
            (*(snap->processo + i))->szExeFile
        );
    }
    return snap->total;
};  // lista_snapshot()


