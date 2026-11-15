#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQUIVO "contas.dat"

typedef enum {
    CONTA_ENCERRADA,
    CONTA_ATIVA
} SituacaoConta;


typedef struct {
    long numero;
    char titular[100];
    char cpf[15];
    double saldo;
    SituacaoConta situacao;
} Conta;


int buscarConta(FILE *arquivo, long numeroConta, Conta *conta);
int atualizarConta(FILE *arquivo, const Conta *conta);
void cadastrarConta(void);
void consultarConta(void);
int depositar(long numeroConta, double valor);
int sacar(long numeroConta, double valor);
int transferir(long origem, long destino, double valor);
void listarContas(void);
int encerrarConta(long numeroConta);


int buscarConta(FILE *arquivo, long numeroConta, Conta *conta) {
    rewind(arquivo);

    while (fread(conta, sizeof(Conta), 1, arquivo) == 1) {
        if (conta->numero == numeroConta) {
            return 1;
        }
    }

    return 0;
}

int atualizarConta(FILE *arquivo, const Conta *conta) {
    Conta auxiliar;

    rewind(arquivo);

    while (1) {
        long posicao = ftell(arquivo);

        if (fread(&auxiliar, sizeof(Conta), 1, arquivo) != 1) {
            return 0;
        }

        if (auxiliar.numero == conta->numero) {
            if (fseek(arquivo, posicao, SEEK_SET) != 0) {
                return 0;
            }

            return fwrite(conta, sizeof(Conta), 1, arquivo) == 1;
        }
    }
}

void cadastrarConta(void) {
    FILE *arquivo = fopen(ARQUIVO, "ab+");
    Conta conta;

    if (arquivo == NULL) {
        printf("Erro ao abrir arquivo!\n");
        return;
    }

    printf("\n=== CADASTRO DE CONTA ===\n");

    printf("Numero da conta: ");
    if (scanf("%ld", &conta.numero) != 1 || conta.numero <= 0) {
        printf("Numero invalido!\n");
        fclose(arquivo);
        return;
    }

    if (buscarConta(arquivo, conta.numero, &conta)) {
        printf("Essa conta ja existe!\n");
        fclose(arquivo);
        return;
    }

    clearerr(arquivo);

    printf("Nome do titular: ");
    scanf(" %99[^\n]", conta.titular);

    printf("CPF: ");
    scanf(" %14s", conta.cpf);

    conta.saldo = 0.0;
    conta.situacao = CONTA_ATIVA;

    fseek(arquivo, 0, SEEK_END);

    if (fwrite(&conta, sizeof(Conta), 1, arquivo) == 1) {
        printf("\nConta cadastrada com sucesso!\n");
    } else {
        printf("Erro ao gravar conta!\n");
    }

    fclose(arquivo);
}

void consultarConta(void) {
    FILE *arquivo = fopen(ARQUIVO, "rb");
    Conta conta;
    long numero;

    if (arquivo == NULL) {
        printf("Nenhuma conta cadastrada.\n");
        return;
    }

    printf("\nNumero da conta: ");
    if (scanf("%ld", &numero) != 1) {
        fclose(arquivo);
        return;
    }

    if (buscarConta(arquivo, numero, &conta)) {
        printf("\n=== DADOS DA CONTA ===\n");
        printf("Numero: %ld\n", conta.numero);
        printf("Titular: %s\n", conta.titular);
        printf("CPF: %s\n", conta.cpf);
        printf("Saldo: R$ %.2f\n", conta.saldo);

        printf("Situacao: %s\n",
            conta.situacao == CONTA_ATIVA ?
            "ATIVA" : "ENCERRADA");
    } else {
        printf("Conta nao encontrada!\n");
    }

    fclose(arquivo);
}

int depositar(long numeroConta, double valor) {
    FILE *arquivo;
    Conta conta;

    if (valor <= 0) {
        return 0;
    }

    arquivo = fopen(ARQUIVO, "rb+");

    if (arquivo == NULL) {
        return 0;
    }

    if (!buscarConta(arquivo, numeroConta, &conta)) {
        fclose(arquivo);
        return 0;
    }

    if (conta.situacao != CONTA_ATIVA) {
        fclose(arquivo);
        return 0;
    }

    conta.saldo += valor;

    int sucesso = atualizarConta(arquivo, &conta);
    fclose(arquivo);

    return sucesso;
}

int sacar(long numeroConta, double valor) {
    FILE *arquivo;
    Conta conta;

    if (valor <= 0) {
        return 0;
    }

    arquivo = fopen(ARQUIVO, "rb+");

    if (arquivo == NULL) {
        return 0;
    }

    if (!buscarConta(arquivo, numeroConta, &conta)) {
        fclose(arquivo);
        return 0;
    }

    if (conta.situacao != CONTA_ATIVA ||
        conta.saldo < valor) {
        fclose(arquivo);
        return 0;
    }

    conta.saldo -= valor;

    int sucesso = atualizarConta(arquivo, &conta);
    fclose(arquivo);

    return sucesso;
}

int transferir(long origem, long destino, double valor) {
    FILE *arquivo;
    Conta contaOrigem, contaDestino;

    if (valor <= 0 || origem == destino) {
        return 0;
    }

    arquivo = fopen(ARQUIVO, "rb");

    if (arquivo == NULL) {
        return 0;
    }

    int encontrouOrigem =
        buscarConta(arquivo, origem, &contaOrigem);

    int encontrouDestino =
        buscarConta(arquivo, destino, &contaDestino);

    fclose(arquivo);

    if (!encontrouOrigem || !encontrouDestino) {
        return 0;
    }

    if (contaOrigem.situacao != CONTA_ATIVA ||
        contaDestino.situacao != CONTA_ATIVA ||
        contaOrigem.saldo < valor) {
        return 0;
    }

    if (!sacar(origem, valor)) {
        return 0;
    }

    if (!depositar(destino, valor)) {
        
        sacar(origem, 0);
        depositar(origem, valor);
        return 0;
    }

    return 1;
}


void listarContas(void) {
    FILE *arquivo = fopen(ARQUIVO, "rb");
    Conta conta;

    if (arquivo == NULL) {
        printf("Nenhuma conta cadastrada.\n");
        return;
    }

    printf("\n=== LISTAGEM DE CONTAS ===\n");

    while (fread(&conta, sizeof(Conta), 1, arquivo) == 1) {
        printf("\nNumero: %ld\n", conta.numero);
        printf("Titular: %s\n", conta.titular);
        printf("CPF: %s\n", conta.cpf);
        printf("Saldo: R$ %.2f\n", conta.saldo);

        printf("Situacao: %s\n",
            conta.situacao == CONTA_ATIVA ?
            "ATIVA" : "ENCERRADA");
    }

    fclose(arquivo);
}

int encerrarConta(long numeroConta) {
    FILE *arquivo = fopen(ARQUIVO, "rb+");
    Conta conta;

    if (arquivo == NULL) {
        return 0;
    }

    if (!buscarConta(arquivo, numeroConta, &conta)) {
        fclose(arquivo);
        return 0;
    }

    if (conta.situacao != CONTA_ATIVA ||
        conta.saldo != 0.0) {
        fclose(arquivo);
        return 0;
    }

    conta.situacao = CONTA_ENCERRADA;

    int sucesso = atualizarConta(arquivo, &conta);

    fclose(arquivo);

    return sucesso;
}

void menuDeposito(void) {
    long numero;
    double valor;

    printf("Numero da conta: ");
    scanf("%ld", &numero);

    printf("Valor do deposito: ");
    scanf("%lf", &valor);

    if (depositar(numero, valor)) {
        printf("Deposito realizado com sucesso!\n");
    } else {
        printf("Nao foi possivel realizar o deposito!\n");
    }
}

void menuSaque(void) {
    long numero;
    double valor;

    printf("Numero da conta: ");
    scanf("%ld", &numero);

    printf("Valor do saque: ");
    scanf("%lf", &valor);

    if (sacar(numero, valor)) {
        printf("Saque realizado com sucesso!\n");
    } else {
        printf("Nao foi possivel realizar o saque!\n");
    }
}

void menuTransferencia(void) {
    long origem, destino;
    double valor;

    printf("Conta de origem: ");
    scanf("%ld", &origem);

    printf("Conta de destino: ");
    scanf("%ld", &destino);

    printf("Valor da transferencia: ");
    scanf("%lf", &valor);

    if (transferir(origem, destino, valor)) {
        printf("Transferencia realizada com sucesso!\n");
    } else {
        printf("Nao foi possivel realizar a transferencia!\n");
    }
}

void menuEncerramento(void) {
    long numero;

    printf("Numero da conta: ");
    scanf("%ld", &numero);

    if (encerrarConta(numero)) {
        printf("Conta encerrada com sucesso!\n");
    } else {
        printf("Nao foi possivel encerrar a conta!\n");
    }
}

int main(void) {
    int opcao;

    do {
        printf("\n========== CAIXA ELETRONICO ==========\n");
        printf("1 - Cadastrar conta\n");
        printf("2 - Consultar conta\n");
        printf("3 - Realizar deposito\n");
        printf("4 - Realizar saque\n");
        printf("5 - Realizar transferencia\n");
        printf("6 - Listar contas\n");
        printf("7 - Encerrar conta\n");
        printf("0 - Sair\n");
        printf("======================================\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Opcao invalida!\n");
            break;
        }

        switch (opcao) {
            case 1:
                cadastrarConta();
                break;

            case 2:
                consultarConta();
                break;

            case 3:
                menuDeposito();
                break;

            case 4:
                menuSaque();
                break;

            case 5:
                menuTransferencia();
                break;

            case 6:
                listarContas();
                break;

            case 7:
                menuEncerramento();
                break;

            case 0:
                printf("Operação cancelada\n");
                break;

            default:
                printf("Opcao invalida!\n");
        }

    } while (opcao != 0);

    return 0;
}