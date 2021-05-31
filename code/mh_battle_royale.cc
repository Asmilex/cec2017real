extern "C" {
    #include "cec17.h"
}
#include <assert.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <functional>


using namespace std;

void clip(vector<double> &sol, int lower, int upper) {
  for (auto &val : sol) {
    if (val < lower) {
      val = lower;
    }
    else if (val > upper) {
      val = upper;
    }
  }
}

void increm_bias(vector<double> &bias, vector<double> dif) {
  for (unsigned i = 0; i < bias.size(); i++) {
    bias[i] = 0.2*bias[i]+0.4*(dif[i]+bias[i]);
  }
}

void decrement_bias(vector<double> &bias, vector<double> dif) {
  for (unsigned i = 0; i < bias.size(); i++) {
    bias[i] = bias[i]-0.4*(dif[i]+bias[i]);
  }
}

/**
 * Aplica el Solis Wets
 *
 * @param sol solucion a mejorar.
 * @param fitness fitness de la solución.
 * @param
 * @param maxevals número máximo de evaluaciones que ejecutará el algoritmo
 * @param lower ínfimo del espacio de búsqueda
 * @param upper máximo del espacio de búsqueda
 *
 * @details Ejemplo de uso: `bl_soliswets(sol, fitness, 0.2, 1000, -100, 100, generador);`
 */
template <class Random>
double bl_soliswets(vector<double> &sol, double &fitness, double delta, int maxevals, int lower, int upper, Random &random) {
  const size_t dim = sol.size();
  vector<double> bias (dim), dif (dim), newsol (dim);
  double newfit;
  size_t i;

  int evals = 0;
  int num_success = 0;
  int num_failed = 0;

  while (evals < maxevals) {
    std::uniform_real_distribution<double> distribution(0.0, delta);

    for (i = 0; i < dim; i++) {
      dif[i] = distribution(random);
      newsol[i] = sol[i] + dif[i] + bias[i];
    }

    clip(newsol, lower, upper);
    newfit = cec17_fitness(&newsol[0]);
    evals += 1;

    if (newfit < fitness) {
      sol = newsol;
      fitness = newfit;
      increm_bias(bias, dif);
      num_success += 1;
      num_failed = 0;
    }
    else if (evals < maxevals) {

      for (i = 0; i < dim; i++) {
        newsol[i] = sol[i] - dif[i] - bias[i];
      }

      clip(newsol, lower, upper);
      newfit = cec17_fitness(&newsol[0]);
      evals += 1;

      if (newfit < fitness) {
        sol = newsol;
        fitness = newfit;
        decrement_bias(bias, dif);
        num_success += 1;
        num_failed = 0;
      }
      else {
        for (i = 0; i < dim; i++) {
          bias[i] /= 2;
        }

        num_success = 0;
        num_failed += 1;
      }
    }

    if (num_success >= 5) {
      num_success = 0;
      delta *= 2;
    }
    else if (num_failed >= 3) {
      num_failed = 0;
      delta /= 2;
    }
  }

  return evals;
}

struct Elemento {
    vector<double> solucion;
    double fitness;

    bool operator<(const Elemento& rhs) const {
        return fitness < rhs.fitness;  //assume that you compare the record based on a
    }

    bool operator== (const Elemento& rhs) const {
        return std::equal(solucion.begin(), solucion.end(), rhs.solucion.begin());
    }
};


auto incrementar_radio (double radio_antiguo) -> double {
    return radio_antiguo + 2.5;
}



template <class Random>
auto generar_nueva_solucion (int dimension, Random &generador) -> vector<double> {
    std::uniform_real_distribution<> uniforme_real(-100.0, 100.0);
    vector<double> sol;

    for (int i = 0; i < dimension; i++) {
        sol.push_back(uniforme_real(generador));
    }

    return sol;
}


/// Genera ciertas soluciones aleatorias y se queda con las mejores
template <class Random>
auto generar_poblacion_inicial (int elementos_poblacion, int aleatorios_a_generar, int dim, Random &generador) -> vector<Elemento> {
    vector<Elemento> poblacion;

    for (int i = 0; i < aleatorios_a_generar; i++) {
        auto sol = generar_nueva_solucion(dim, generador);

        poblacion.push_back({
            .solucion = sol,
            .fitness = cec17_fitness(&sol[0])
        });
    }

    sort(poblacion.begin(), poblacion.end());

    if (aleatorios_a_generar > elementos_poblacion) {
        poblacion.erase(poblacion.begin() + elementos_poblacion, poblacion.end());
    }

    assert(poblacion.size() == elementos_poblacion);

    return poblacion;
}


template <typename T>
double distancia (const std::vector<T>& a, const std::vector<T>& b) {
	std::vector<double>	auxiliary;

	std::transform (
        a.begin(),
        a.end(),
        b.begin(),
        std::back_inserter(auxiliary),
        [](T element1, T element2) {
            return pow((element1-element2), 2);
        }
    );

	return std::sqrt(std::accumulate(auxiliary.begin(), auxiliary.end(), 0.0));
}


int main(int argc, char *argv[]) {
    int dim = 10;

    if (argc > 1) {
        dim = atoi(argv[1]);
    }

    std::uniform_real_distribution<> uniforme_real(-100.0, 100.0);

    const int poblacion_inicial = 20;
    const int aleatorios_a_generar = 100;

    const int periodo_generacional = 3;    // Cada x número de generaciones, comprobar la distancia entre los elementos.
    const int evaluaciones_bl_maximas = 100;
    const double delta = 0.4;


    for (int id_funcion = 1; id_funcion <= 30; id_funcion++) {
        cec17_init("MH_battle_royale", id_funcion, dim);

        std::mt19937 generador;

        vector<Elemento> poblacion = generar_poblacion_inicial(poblacion_inicial, poblacion_inicial, dim, generador);
        int evaluaciones = aleatorios_a_generar;
        double radio = 0.1;

        // Generar población inicial.
        // Generamos ciertas soluciones iniciales, y nos quedamos con las `poblacion_inicial mejores`.

        int t = 1;
        while (evaluaciones < 10000 * dim) {
            if ((t+1) % periodo_generacional == 0) {
                cout << "F["<< id_funcion << "] Generación " << t << ". Elementos restantes: " << poblacion.size() << ". Radio: " << radio <<  ". Evaluaciones totales: " << evaluaciones << ". Fitness: " << poblacion[0].fitness << endl;
            }

            if (t % periodo_generacional == 0) {
                vector<Elemento> conjunto_temporal (poblacion);     // Para evitar repetidos

                // Comprobar aquellos que se encuentran cerca
                for (int i = 0; i < poblacion.size(); i++) {
                    for (int j = 0; j < poblacion.size(); j++) {
                        if (i != j && distancia(poblacion[i].solucion, poblacion[j].solucion) < radio) {
                            //cout << "Elementos cercanos: " << i << ", " << j << endl;
                            // Están suficientemente cerca => nos cargamos la peor.
                            Elemento peor_sol;

                            if (poblacion[i] < poblacion[j]) {
                                peor_sol = poblacion[j];
                            }
                            else {
                                peor_sol = poblacion[i];
                            }

                            conjunto_temporal.erase(
                                std::remove_if(
                                    conjunto_temporal.begin(),
                                    conjunto_temporal.end(),
                                    [&peor_sol](Elemento const & s) {
                                        return s == peor_sol;
                                    }
                                ),
                                conjunto_temporal.end()
                            );
                        }
                    }
                }

                poblacion.clear();
                poblacion = conjunto_temporal;

            }

            radio = incrementar_radio(radio);

            // Aplicar la BL a cada una de nuestras soluciones
            for (int i = 0; i < poblacion.size(); i++) {
                auto fitness_antiguo = poblacion[i].fitness;

                evaluaciones += bl_soliswets(
                    poblacion[i].solucion,
                    poblacion[i].fitness,
                    delta,
                    evaluaciones_bl_maximas,
                    -100,
                    100,
                    generador
                );

                poblacion[i].fitness = cec17_fitness(&poblacion[i].solucion[0]);
                evaluaciones++;

                //cout << "Sol " << i << ": fitness viejo = " << fitness_antiguo << "; nuevo = " << poblacion[i].fitness << endl;
            }

            t++;
        }

        sort(poblacion.begin(), poblacion.end());
        auto mejor = poblacion[0].fitness;

        cout <<"Mejor solución [F" <<id_funcion <<"]: " << scientific <<cec17_error(mejor) <<endl;
    }
}