---
title: Optimización de la búsqueda multiarranque básica usando las ideas de los juegos de tipo Battle Royale
author: Andrés Millán
subject: Metaheurísticas
keywords: búsqueda local, MH, metaheurísticas, optimización
---

# Optimización de la búsqueda multiarranque básica usando las ideas de los juegos de tipo Battle Royale

> Autor: Andrés Millán
> Correo: amilmun@ugr.es

## Preámbulos

En los últimos años, se ha puesto de moda inspirarse en los procesos naturales para crear una metaheurística. Esta forma de trabajar ha sido criticada por el medio, pues no garantiza que se consigan algoritmos competentes ni mejore el estado del arte.

Ignorando esta crítica, nosotros intentaremos producir una nueva usando como base un tipo de población: la de humanos pasándoselo bien.

* * *

## Los juegos del género battle royale

En la era moderna de los juegos competitivos online, ha surgido un género denominado **battle royale**.

En ellos, varios jugadores o equipos son lanzados en un mapa, de forma que pueden recorrerlo libremente. Al ser un juego competitivo, deben luchar entre ellos, y quien quede en pie al final, ganará.
Los diseñadores emplean un método al que se le suele llamar **anillos** para darle emoción. Tras cierto tiempo, y dependiendo de la ronda, se reduce el terreno de juego mediante una zona de radio decreciente. Poco a poco, va atrapando a los equipos, de forma que se ven obligados a ir hacia el centro del mapa, lo que incrementa las probabilidades de que surjan enfrentamientos.

Para ejemplificar esto, usaremos el juego llamado **Apex Legends**.

Al principio de la batalla, los jugadores se encuentran repartidos por el mapa en zonas de interés (debido al equipamiento):
<img align="center" src="./img/ApexR1.jpg" alt="Apex ronda 1">

En las siguientes rondas, vemos que el anillo se cierra y propicia encuentros:
<img align="center" src="./img/ApexR4.png" alt="Apex ronda 4" width="250">

<img align="center" src="./img/ApexR7.jpg" alt="Apex ronda 7">

* * *

## Adaptándolo a una metaheurística

Este comportamiento resulta muy interesante. Podemos intentar extrapolar la idea de *anillo*, y aplicársela a un conjunto de soluciones de una metaheurística.

En primera instancia, mi idea inicial era dividir el espacio de búsqueda en sectores, de forma que se forzara a combatir a dos soluciones cuando se encontraban en el mismo. Sin embargo, esta idea resulta difícil de implementar, es costosa (pues escala muy mal con las dimensiones), y podría generar absurdos (dos soluciones muy cercanas que no combatan).

Por ello, simularemos los anillos a la inversa: de cada solución partirá un anillo, que irá creciendo conforme pasen las generaciones. Al principio, será una bola n-dimensional pequeña. Al tiempo, aumentará su radio, lo que hará que se encuentre con otras soluciones. Cuando esto ocurra, las haremos combatir. La peor de las dos morirá, y la otra seguirá avanzando.

<img align="center" src="./img/Dibujo_anillos.png" alt="Dibujo anillos">

Al final, quedará una única solución (o varias, quedándonos con la mejor), que será la vencedora del algoritmo.

Con este planteamiento, es muy importante acertar con el esquema evolutivo de la población. Una mala elección de los parámetros puede hacer que esta metaheurística no tenga ningún sentido. Por ejemplo, una tasa baja de crecimiento de los anillos hará que no se combata nunca. O una búsqueda local muy exhaustiva agotará todas las evaluaciones de las que
disponemos.

## Implementación inicial

Una vez hemos planteado las ideas básicas del algoritmo, es hora de pasar a la implementación.

Para la búsqueda local, usaremos la proporcionada por Daniel Molina: la Solis-Wets. Además, debemos definir nuestra función de aumento de radio. Es importante encontrar un buen equilibrio entre el crecimiento del radio y el número de evaluaciones. Si aumentamos demasiado rápido el radio, la población morirá pronto. Si se limita su crecimiento, las soluciones nunca combatirán.
Como vamos a limitar la BL a 100 evaluaciones por ejecución, en esta primera versión usaremos `radio_antiguo + k`, con `k = 2`. De momento, esto funciona suficientemente bien. Se mantiene el equilibrio que buscábamos de momento. Más tarde investigaremos si merece la pena cambiarla.

El pseudocódigo sería similar al siguiente:

```
// ──────────────────────────────────────────────────────────────── VERSION 1 ─────
/*
    Se asume una estructura del tipo {
        solucion,
        fitness
    },
    que tiene implementados comparadores de igualdad y de orden
*/

incrementar_radio (radio_antiguo) {
    return radio_antiguo + 2
}


MH_battle_royale () {
    // ─────────────────────────────────────────────────────────── PARAMETROS ─────

    dim = 10

    generador: Random con distribución uniforme en (-100, 100)
    semilla = ...

    poblacion_inicial = 20
    periodo_generacional = 3
    evaluaciones_maximas_bl = 100
    delta = 0.4
    radio = 0.1

    poblacion = generar_poblacion_inicial(poblacion_inicial, dim, generador)
    evaluaciones = poblacion_inicial

    // ────────────────────────────────────────────────────── BUCLE PRINCIPAL ─────

    t = 1

    while (evaluaciones < 10000 * dim) {
        if (t % periodo_generacional == 0) {
            Comprobar si en `poblacion` existen dos o más elementos con distancia entre ellos menor que radio.
            Si es así, eliminar la(s) que tenga(n) peor fitness.
        }

        radio = incrementar_radio(radio)

        // Aplicar BL a las soluciones
        for (i in 0..poblacion.size()) {
            evaluaciones += bl_soliswets(
                poblacion[i].solucion,
                poblacion[i].fitness,
                delta,
                evaluaciones_bl_maxiams,
                -100,
                100,
                generador
            )

            poblacion[i].fitness = fitness(poblacion[i].solucion)
            evaluaciones++
        }

        t++
    }

    poblacion.sort()

    return poblacion[0]     // Contiene solución y fitness.
}
```