#pragma once
/*
* Arquivo: efem.h
* Descrição: Definição da classe Efem
* Data: 12 Ago 2005
* Autor: José Marcelo Lima Duarte
*/

#ifndef EFEM_H
#define EFEM_H

#include <ctime>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

class Efem {

public:

    // Construtor -  Inicializa os dados privatos com valores consistentes
    Efem();

    // Destruidor
    ~Efem();

    // Retorna o horario, azimute e a elevação da posição atual na tabela
    // Os valores são passados por referencia
    void get_efem(time_t *ef_time, float *azi, float *ele, float *range) const;	// ( 0.00 - 360.00 ) (0.00 - 90 )

    // Avança uma posição na tabela e retorna 0.
    // Retorna 1 se nao for possivel por ultrapassar o fim da tabela;
    int next();

    //função que define o nome do satelite
    void set_name(char *_sat_name);

    //Função que armazenará os dados da passagem (hora inicial e final, duração e maxima elevação)
    void set_data(time_t _start_t, time_t _stop_t, float _duration, float _max_ele);

    //Função que retorna os dados da passagem (hora inicial e final, duração, maxima elevação e o nome do satelite)
    void get_info(time_t *_start_t, time_t *_stop_t, float *_duration, float *_max_ele, char *_arq_name);

    //armazena as informações das efemérides (data e hora, azimute, elevação, range e um
    //auxiliar indicando posição da efemeride atual)
    void set_efem(time_t _d_ef_time, float _az, float _ele, float _range, int _cont_efem);

    // Verifica se a antena vai passar pelo 0 graus azimute
    void change_efem();

    //Sobrecarga do operador >       Ex: Efem::passagem[a] > Efem::passagem[b]
    //Irá verificar se a passagem 'a' ocorre primeiro que a passagem 'b'. Caso ocorra
    //retornará true, caso não retornará false.
    bool operator>(Efem b);

    //Sobrecarga do operador =       Ex: Efem::passagem[a] = Efem::passagem[b]
    //Irá igualar todos os atributos do objeto 'a' ao objeto 'b'
    void operator=(Efem b);


private:

    // Transforma a coordenada com elevação entre 0 e 90 para
    // a coordenada equivalente com elevação entre 90 e 180
    // Ex: az: 0  ele: 30  =  az: 180  ele: 150 (180-30)
    void muda_efe();
    //funcao para mudanca de referencia
    void change_ref(float *az, float *ele);

    char sat_name[20]; //Array que armazenará o nome do satelite
    time_t d_ef_time[1200];	// armazena o horario das coordenadas
    float d_azi[1200];	// armazena as coordenadas azimutes (0 - 360)
    float d_ele[1200];	// armazena as coordenadas elevacao (0 - 90)
    float d_range[1200];	// armazena o range do satelite a estação
    int d_pos;			// linha atual na tabela das efemerides (0, 1, 2, ... , tam_da_tabela - 1 )
    int d_size; 		// numero de linhas da tabela das efemerides
    float d_offset_az, d_offset_ele; //armazena os offsets para cada posicao
    time_t start_t, stop_t; //Horario de inicio e fim das passagens
    float duration; //Duração da passagem
    float max_ele; //Máxima elevação da antena na passagem atual
};

#endif
