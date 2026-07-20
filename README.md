# Drivers YF-S201 Water Flow Sensor

**Para ATmega328P**

## Funcionamiento del sensor

Para la programación de este software primero debo investigar el sensor de caudal YF-S201, su modo de funcionamiento y cómo trabajar con él.

Este sensor posee una turbina a través de la cual circula una corriente de fluido. En una de las aspas de esta turbina se encuentra un imán que al rotar genera cambios en un campo magnético fruto del efecto Hall. Cada giro de la turbina produce pulsos eléctricos en la salida del sensor, siendo la frecuencia de estos pulsos proporcional al caudal circulando por el sensor (en el rango de funcionamiento).

Para poder usar este sensor en nuestro sistema debemos poder medir la frecuencia de los pulsos de salida, para eso tendremos en cuenta la información de la hoja de datos del YF-S201:

- **Working Flow Rate:** 1 a 30 Litros/Minuto
- **Output duty cycle:** 50% ±10%
- **Flow rate pulse characteristics:** Frequency (Hz) = 7.5 × Flow rate (L/min)

Estas tres especificaciones nos dicen: el rango de correcto funcionamiento del sensor, fuera de ese rango el sensado podría no ser confiable; el tipo de señal de salida, que será una señal aproximadamente cuadrada; y la constante de proporción entre frecuencia de salida del sensor y caudal medido, que es de 7.5 Hz por L/min.

Teniendo esto en cuenta, nuestro objetivo será poder medir correctamente la frecuencia de la señal de salida del sensor, y con ese valor determinar el caudal actual circulando por el sistema.

## Interpretando la salida del sensor

Para medir la frecuencia de la señal de salida del sensor YF-S201 usaré el Input Capture del Timer1 incorporado en el microcontrolador, ubicado el pin PB0 (pin 8 del Arduino UNO). Esta implementación me permite configurar la frecuencia de conteo del Timer 1, para así en cada flanco de subida de la señal del sensor muestrear el contador interno del periférico y compararlo con el valor de la muestra anterior, pudiendo calcular así la diferencia entre estos dos valores y, por lo tanto, el tiempo entre muestra y muestra, que se traduce como el período de la señal.

La frecuencia que estaremos midiendo usando esta herramienta tendrá la siguiente expresión:

```
f = Fcpu / (N * ΔICR1)
```

Donde la frecuencia del microcontrolador es de 16 MHz, `N` es el preescalador del Timer 1 y `ΔICR1` es la diferencia del contador entre muestras.

Lo importante de esta expresión será que para el rango de trabajo `ΔICR1` debe ser menor a 2^16, para así poder calcular correctamente el tiempo entre muestras sin tener que preocuparnos por el desbordamiento del contador del Timer. Para asegurarnos de esto calculamos su valor en los casos límite:

**f (1 L/min):**
```
7.5 Hz = Fcpu / (64 * ΔICR1)  ⇒  ΔICR1 = 33333.33
```

**f (30 L/min):**
```
225 Hz = Fcpu / (64 * ΔICR1)  ⇒  ΔICR1 = 1111.11
```

Vemos que ambos límites cumplen la condición anterior, pero surge un problema nuevo: el error. Para poder medir perfectamente estas frecuencias necesitamos poder muestrear "entre" cuentas del Timer, lo que no es posible; por eso elegimos un preescalador chico (64), porque si no podemos muestrear "entre" cuentas, por lo menos que estas cuentas sean lo más rápidas posibles para que el error sea lo menor posible. Este error será:

**f (1 L/min):**
```
Fcpu / (64 * 33333) = 7.500075 Hz  ⇒  e = 0.001%
```

**f (30 L/min):**
```
Fcpu / (64 * 1111) = 225.0225023 Hz  ⇒  e = 0.010001%
```

Vemos que dentro del rango de trabajo el error es mínimo, por lo que el sistema es fiable.

Teniendo esto en cuenta, encontramos una manera de poder medir la frecuencia de la señal de salida del sensor y así poder calcular el caudal instantáneo que por este circula. La expresión será la siguiente:

```
Q[L/min] = Fcpu / (64 * ΔICR1 * 7.5 Hz) = (1 / ΔICR1) * (100000 / 3)
```

## Detalles adicionales

Adicionalmente, cuando por el sensor no circula fluido (caudal cero) este no genera pulsos, por lo que el sistema mantiene el último valor de caudal calculado y no actualiza el cambio de valor, dando una medición de caudal errónea. Es por esto que usando el periférico Timer 0 agregué un sistema de timeout que detecta cuando el sensor dejó de generar pulsos por un tiempo determinado, y así actualizar el valor del caudal a cero. El tiempo de timeout es de 150 ms, ya que el mayor período producido por el sensor es el correspondiente a Q = 1 L/min ⇒ T = 133.33 ms.

## Conexionado y programación

El sensor consta de 3 cables:

- **Rojo:** VCC.
- **Negro:** GND.
- **Amarillo:** señal de salida.

Para poder medir la señal de salida del sensor usando Input Capture, el cable correspondiente deberá conectarse al pin correspondiente para esta herramienta, el PB0, el cual el propio driver configura como Input Pull-up, ya que el sensor actúa poniendo la señal en bajo.

Para el funcionamiento del Input Capture, el Timer 1 será programado:

- Modo normal.
- Preescalador 64.
- Captura en flanco ascendente.
- Filtro de ruido.

Para el sistema de timeout, el Timer 0 interrumpirá cada 1 ms, por lo que será programado:

- Modo CTC.
- Prescaler 64.
- OCR0A = 249.
