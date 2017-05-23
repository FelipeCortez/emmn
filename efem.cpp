#include "efem.h"

// Construtor
// inicializa os dados privados com valores consistentes
Efem::Efem() {
    d_azi[0] = 180;
    d_ele[0] = 90;
    d_range[0] = 0;
    d_pos = 0;
    d_size = 0;
    d_offset_az = -19.0;
    d_offset_ele = 0.0;
    duration = 0;
    max_ele = 0.0;
}

// Destruidor
Efem::~Efem() {}

// Passa por referencia o horario, azimute e a elevação da posição atual na tabela
// Azimute (0.00 360.00), Elevação (0.00 90.00)
void Efem::get_efem(time_t *ef_time, float *azi, float *ele, float *range) const {
    *ef_time = d_ef_time[d_pos];
    *azi = d_azi[d_pos];
    *ele = d_ele[d_pos];
    *range = d_range[d_pos];
}

// Avança uma posição na tabela e retorna 0.
// Retorna 1 se nao for possivel por ultrapassar o fim da tabela;
int Efem::next() {
    if (d_pos < (d_size - 1)) {
        d_pos++;
        return 0;
    }
    else {
        return 1;
    }
}

//Define o nome do satelite da passagem atual
void Efem::set_name(char *_sat_name)
{
    //int lixo = 0;
    for (int i = 0; i < 7; i++) {
        sat_name[i] = _sat_name[i];
    }
    if (sat_name[0] == 'S' || sat_name[0] == 's') {
        sat_name[4] = NULL;
        sat_name[5] = NULL;
        sat_name[6] = NULL;
    }
}

//Define os dados basicos da passagem atual (hora inicial e final, duração e maxima elevação)
void Efem::set_data(time_t _start_t, time_t _stop_t, float _duration, float _max_ele)
{
    start_t = _start_t;
    stop_t = _stop_t;
    duration = _duration;
    max_ele = _max_ele;
}

//Passa por referência os dados da passagem atual (hora inicial e final, duração e maxima elevação)
void Efem::get_info(time_t *_start_t, time_t * _stop_t, float * _duration, float * _max_ele, char *_arq_name)
{
    if (sat_name[0] == 'S') {
        for (int i = 0; i < 4; i++) {
            _arq_name[i] = sat_name[i];
        }
    }
    else
    {	for (int i = 0; i < 7; i++) {
            _arq_name[i] = sat_name[i];
        }
    }

    *_start_t = start_t;
    *_stop_t = stop_t;
    *_duration = duration;
    *_max_ele = max_ele;
}

//Adiciona uma efemeride na passagem atual e incrementa o tamanho da tabela
void Efem::set_efem(time_t _d_ef_time, float _az, float _ele, float _range, int _cont_efem)
{
    d_ef_time[_cont_efem] = _d_ef_time;
    d_azi[_cont_efem] = _az;
    d_ele[_cont_efem] = _ele;
    d_range[_cont_efem] = _range;
    d_size++;

}

//Esta funcao retira o OFFSET magnetico no sistema de coordenadas da antena
void Efem::change_ref(float *az, float *ele) {

    int cont = 0;

    for (cont = 0; cont < d_size; cont++) {
    az[cont] = az[cont] + d_offset_az;
    ele[cont] = ele[cont] + d_offset_ele;
    if (az[cont]<0)az[cont] = az[cont] + 360;
    if (az[cont] >= 360)az[cont] = az[cont] - 360;
    }
}

// Realiza os ajustes necessários nas efemérides, como offset
// magnético, e passagem pelo final de curso
void Efem::change_efem() {
    //Muda a referência das efemérides acrescentando o offset magnético
    change_ref(d_azi, d_ele);

    //Verifica se o azimute passa pela descontinuidade no 0°
    //Se passar, faz o ajuste necessário nas referências
    if (fabs(d_azi[d_size - 1] - d_azi[0]) > 180.00) {
        muda_efe();
    }
}

//Sobrecarga do operador >
//Retorna verdadeiro se a hora de inicio da passagem A for maior que
//O horario de inicio da passagem B. Caso contrario, retorna falso
bool Efem::operator>(Efem b)
{
    if (start_t > b.start_t) {
        return true;
    }
    else {
        return false;
    }
}

//Sobrecarga do operador =
//Iguala todos os parametros da passagem A aos parametros da
//passagem B.
void Efem::operator=(Efem b)
{
    int cont=0;

    for (int i = 0; i < 7; i++) {
        sat_name[i] = b.sat_name[i];
    }

    d_pos = b.d_pos;
    d_size = b.d_size;
    d_offset_az = b.d_offset_az;
    d_offset_ele = b.d_offset_ele;
    start_t = b.start_t;
    stop_t = b.stop_t;
    duration = b.duration;
    max_ele = b.max_ele;

    for (cont = 0; cont < b.d_size; cont++) {
        d_ef_time[cont] = b.d_ef_time[cont];
        d_azi[cont] = b.d_azi[cont];
        d_ele[cont] = b.d_ele[cont];
        d_range[cont] = b.d_range[cont];
    }
}


// Transforma a coordenada com elevação entre 0 e 90 para
// a coordenada equivalente com elevação entre 90 e 180
void Efem::muda_efe() {
    int i = 0;
    for (i = 0; i < d_size; i++) {
        d_azi[i] = d_azi[i] - 180;
        if (d_azi[i] < 0) {
            d_azi[i] = d_azi[i] + 360;
        }
        d_ele[i] = 180 - d_ele[i];
    }
}
