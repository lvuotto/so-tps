function () {
  if (this.total_votes >= 2000)
    emit("mas_de_2000", {
      arreglo: [ {
        titulo: this.title,
        puntaje: this.score,
      } ]
    });
}
