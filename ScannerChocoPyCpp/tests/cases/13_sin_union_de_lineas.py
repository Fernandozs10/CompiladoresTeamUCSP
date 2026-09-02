# En ChocoPy NO hay union de lineas (Reference 3.1.2): una linea logica
# ES una linea fisica. Un corchete/parentesis abierto NO suprime el
# NEWLINE. El scanner emite NEWLINE al final de la linea 6 e INDENT en
# la 7. Es lexicamente valido; el parser lo rechazaria mas adelante.
xs: [int] = None
xs = [1,
      2]
