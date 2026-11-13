# [Fragen für Abnahme](https://numsim-exercises.readthedocs.io/en/latest/exercise1/submission.html#questions-for-the-interview-abnahme)

### Was passiert, wenn die Reynolds-Zahl geändert wird: Re = 100, 500, 2000, 10000?

* *Antwort:* Je größer die Reynolds-Zahl ist, desto stärker rückt der Wirbel an den rechten Rand und konzentriert sich auf eine bestimmte Stelle. Bei hohen Werten stellt sich die stationäre Strömung schneller ein. Die Maximalgeschwindigkeit ($u_{max}, v_{max}$) bleibt aufgrund der Randbedingungen unverändert. \[24]
* *Zusatzbeobachtung:* Es bilden sich große Wirbel in der Ecke. \[20b] Bei sehr kleinen Reynolds-Zahlen könnten theoretisch weitere Wirbel entstehen, allerdings ist die Gitterauflösung in der Regel zu rudimentär, um diese vollständig aufzulösen. \[24]
* *Dynamik:* Wirbel bilden sich bei größerer Reynolds-Zahl schneller aus, kommen aber bei kleinerer Reynolds-Zahl schneller zum stationären Zustand. Die Strömung selbst ist nicht schneller, da die Maximalgeschwindigkeit durch die Randbedingung fixiert ist. \[21]
* TODO: vllt plots?

### Wählen Sie die Zeitschritte manuell aus. Für welche Zeitschritte dt ist der Algorithmus stabil?

* *Antwort:* Das Programm ist sehr robust und schwer zum Absturz zu bringen. \[21]
* Mit einer festen Zeitschrittweite von $\Delta t = 0.2\text{ s}$ ist das Verfahren im *lid-driven-cavity*-Fall akkumulieren sich Fehler im Druck und das Verfahren schlägt fehl. Eine Schrittweite von $\Delta t = 0.1\text{ s}$ liefert akzeptable, aber nicht 100 % stabile Ergebnisse. Mit $\Delta t < 0.05$ (Einhaltung der CFL-Bedingung) arbeitet das Verfahren stets stabil. \[24]
* Wird der Zeitschritt zu groß gewählt, führt dies zu Artefakten, Flecken und Störungen in der Lösung. \[21]
* *Einfluss von* *$u_{max}/v_{max}$:* Der Sicherheitsfaktor sorgt dafür, dass man im stabilen Bereich bleibt und die CFL-Bedingung erfüllt.
* TODO: vllt plots?

### Wir haben in der Vorlesung festgestellt, dass unsere räumliche Diskretisierung (bei Verwendung zentraler Differenzen) genau zweiter Ordnung/O(h^2) bzw. (bei Verwendung der Donor-Cell-Verfahren) genau erster Ordnung/O(h) ist. Die zeitliche Diskretisierung ist genau erster Ordnung/O(dt). Wie könntet ihr dies mit eurem Simulationsprogramm überprüfen?

* **Raumkonvergenz:** Gitterweite $h$ sukzessive halbieren (z. B. $N_x \in \{32, 64, 128\}$) bei konstant gehaltenem, sehr kleinem Zeitschritt (damit der Zeitfehler das Ergebnis nicht verfälscht). Die Differenz zweier aufeinanderfolgender Lösungen wird auf einer Log-Log-Skala über der Gitterweite aufgetragen. Die Steigung der Geraden entspricht der Konvergenzordnung (z. B. Steigung 1 für Verfahren 1. Ordnung wie Donor Cell, Steigung 2 für zentrale Differenzen). \[20a, 20b, 21, 24]
* **Zeitkonvergenz:** Gitter konstant halten und $\Delta t$ halbieren. dann plotten wie bei der raumkonvergenz. steigung sollte 1 sein für O(dt). \[20b]

### Stellen Sie die Laufzeiten für verschiedene Gitterauflösungen in einer Tabelle zusammen (bringen Sie diese zum Treffen mit, eine pro Gruppe). Stimmen die Ergebnisse mit unseren theoretischen Überlegungen aus der Vorlesung überein? Warum / Warum nicht?

laut skript: wenn\*\*$h \to h/2$\*\* **bzw.** **$N \to 2N$** wird, braucht SOR 2x so viele iterationen (p.29)

meine daten (table\_batches.txt):  h -> h/2 (z.b. von 10x10 -> 20x20) ist zeit 4.314x ???

* FRAGEN: ist das weil wir 2d sind? also ich halbiere h\_x und h\_y jeweils?
* FRAGEN: ist das im skript auch für 2D? ist das im skript für dx=dy=h? (square grids in 2d?)

**Wie ändert sich die Laufzeit des Programms, wenn die Gitterbreite halbiert wird ($h \to h/2$** **bzw.** **$N \to 2N$)?**

* *Antwort:*
  * Zeitschritte verdoppeln sich bis zu $\times 4$. \[20a]
  * SOR-Iterationen / Gauss-Seidel-Iterationen im Pressure-Solver vervierfachen sich ($\times 4$ bzw. $\times 2$ pro Level). \[20a, 24]
  * Insgesamt skaliert die Laufzeit im Worst-Case bis zum $\times 64$-fachen (theoretisch oft $\mathcal{O}(1/h^2)$ bis $\mathcal{O}(n^4)$ bei Berücksichtigung aller Dimensionen und Zeitschritte). \[20a, 24]
* *Beobachtete Laufzeiten aus Tabelle (Beispiel* *$20\times 20$* *bis* *$80\times 80$):*
  * $20 \times 20$: $0.88\text{ s}$ \[24]
  * $20 \times 40$: $2.45\text{ s}$ \[24]
  * $40 \times 40$: $9.99\text{ s}$ \[24] (NOTE: (11.35x vom 20x20 wert von 0.88s)
  * $40 \times 80$: $39.56\text{ s}$ \[24]
  * $80 \times 80$: $196.45\text{ s}$ \[24]
  * *Fazit:* Die Laufzeit wird durch den SOR- bzw. Gauss-Seidel-Solver dominiert; die lineare Komplexität anderer Komponenten (Finite Differenzen) ist vernachlässigbar. \[24]

***

## 2. Zeitschrittweite, CFL-Bedingung & Simulationsdauer

* **Könnte man nicht einfach allgemein einen festen Zeitschritt wählen (z. B.** **$\Delta t = 10^{-5}$)?**
  * *Antwort:* Das kommt auf die Parameter an (insbesondere auf das Verhältnis $\Delta x / \Delta y$). Ein allgemeiner, sehr kleiner Zeitschritt funktioniert, wenn er die CFL-Bedingung erfüllt, ist aber aus Performance-Perspektive unvorteilhaft. Variable Zeitschrittweiten sind zu bevorzugen, da sie zu weniger notwendigen Iterationen führen. \[21, 24]

* **Warum macht es keinen Sinn, Simulationen zu lange laufen zu lassen?**
  * *Antwort:* Das Programm erreicht nach einer gewissen Zeit einen stationären Zustand (Gleichgewichtslösung), in der sich zeitlich nichts mehr ändert. Solange keine Störungen auftreten, verbraucht eine längere Simulationszeit nur unnötig Rechenressourcen. \[21, 24]

***

## 3. Algorithmus für einen Zeitschritt

* **Beschreibe den Algorithmus für einen Zeitschritt.**
  * *Antwort:* Der genaue Ablauf gliedert sich in folgende Schritte:
    1. Randwerte setzen (Boundary values). \[24]
    2. Randwerte für $F$ und $G$ setzen. \[24]
    3. Zeitschrittweite $\Delta t$ berechnen. \[24]
    4. $F$ und $G$ berechnen. \[24]
    5. Rechte Seite (RHS) für die Poisson-Gleichung berechnen. \[24]
    6. Drucklöser-Schleife (`while iter < itmax and res > epsilon`):
       * Eine SOR-Iteration für die Poisson-Gleichung durchführen. \[20a, 24]
       * Randwerte für den Druck $p$ nach jeder Iteration aktualisieren. \[20b, 24]
       * Residuum berechnen. \[24]
    7. Geschwindigkeiten $u$ und $v$ aktualisieren. \[24]
    8. Ausgabe von $u$, $v$ und $p$ (z. B. für Paraview). \[24]
  * *Hinweis zur Laufzeit:* Der SOR-Solver dominiert die Laufzeit des Programms. \[20a]

***

## 4. Randbedingungen (Boundary Conditions) und Update-Logik

* **An welchen Stellen im Programm und wie oft müssen Randbedingungen gesetzt werden?**
  * *Antwort:*
    * Die Randwerte für Geschwindigkeiten $u, v$ sowie $F, G$ werden zu Beginn jedes Zeitschritts (vor den finiten Differenzen) gesetzt, da indirekt auf Mittelwerte am Rand zugegriffen wird. \[20a, 20b, 21, 24]
    * Die Randwerte für den Druck $p$ müssen in *jeder* Iteration des Pressure-Solvers (nach jedem SOR-Schritt) aktualisiert werden. \[20a, 20b, 21, 24]

* **Warum könnte man** **$F$** **und** **$G$** **beim Setzen der Randwerte auch weglassen?**
  * *Antwort:* Bei der Berechnung des Drucks werden $F$ und $G$ am Rand in keinem Fall ausgewertet; stattdessen werden direkt die Randwerte von $u$ und $v$ genutzt. Es ist dennoch sinnvoll, sie zu setzen, falls man andere Szenarien berechnen möchte, in denen sie benötigt werden. \[21, 24]

***

## 5. Gitterstruktur, Feldgrößen und Staggered Grid

* **Wie groß wurden die Field-Variables ($u, v, p$) gewählt und wie begründet sich das?**
  * *Antwort:*
    * Druck $p$: Dimension $(n_x + 2) \times (n_y + 2)$ (analog zur rechten Seite rhs). \[20a, 21, 24]
    * Geschwindigkeiten $u$: Dimension $(n_x + 1) \times (n_y + 2)$ (analog zu $F$). \[21, 24]
    * Geschwindigkeiten $v$: Dimension $(n_x + 2) \times (n_y + 1)$ (analog zu $G$). \[21, 24]
  * *Implementierungsstrategie:* Man kann die Array-Größen direkt von der Skizze des *staggered grid* ablesen und die Start-/Endwerte für Iterationen exakt bestimmen. Manchmal werden Arrays der Einfachheit halber direkt in Druckgröße anlegt, da das minimalen Speicherplatz kostet und die Implementierung vereinfacht. \[20b, 21]

* **Welchen Vorteil hat ein** ***staggered grid*** **gegenüber einem** ***collocated grid***\*\*?\*\*
  * *Antwort:* Bei einem *collocated grid* liegen alle diskreten Werte im Zellenzentrum (alle Variablen bräuchten $n+2$). Das *staggered grid* verhindert physikalisch unphysikalische Druckoszillationen, die auftreten könnten, wenn alle Werte an exakt demselben Punkt ausgewertet würden. \[24]

* **Welche Zellen müssen für die Berechnung von** **$F$** **und** **$G$** **betrachtet werden?**
  * *Antwort:* Für die finiten Differenzen benötigt man die Nachbarzellen (oben/unten, links/rechts) sowie aufgrund gemischter Ableitungen auch die diagonalen Zellen (links-oben für den $G$-Term und rechts-unten für den $F$-Term). \[24]

***

***

## 8. Druckgleichung, Massenerhaltung & Implizite Verfahren

* **Was erfüllt die Druckgleichung physikalisch?**
  * *Antwort:* Sie erfüllt die Massenerhaltung (Inkompressibilität $\nabla \cdot \mathbf{u} = 0$). \[21, 24]

* **Warum dürfen wir den Druckgradienten nicht zum Zeitschritt** **$t^{(n)}$** **explizit auswerten ($u_{i,j}^{(n+1)} = u_{i,j}^{(n)} - \delta t A_{i,j}^{(n)} - \delta t \left[\frac{\partial p}{\partial x}\right]_{i,j}^{(n+1)}$)?**
  * *Antwort:* Die explizite Euler-Methode wird nicht auf den Druck angewendet, sondern nur auf die Konvektions- und Diffusionsterme ($A_{i,j}$). Der Druck wird implizit durch das Geschwindigkeitsfeld bestimmt und steht im ersten Zeitschritt nicht direkt zur Verfügung. Durch die implizite Verwendung hängt die neue Geschwindigkeit direkt vom aktuellen Druck ab, was eine höhere Genauigkeit und die Erfüllung der Massenerhaltung ermöglicht. \[24]

* **Könnte man auch ein implizites Verfahren anwenden, bei dem die Druckgleichung nicht extra betrachtet werden muss?**
  * *Antwort:* Die Druckgleichung müsste dennoch berücksichtigt werden. Man könnte zwar theoretisch ein riesiges gekoppeltes Gleichungssystem für $u, v$ und $p$ (Größe $3N \times 3N$) lösen, dies wäre jedoch deutlich langsamer und speicherintensiver. \[21]

***

## 9. Fehlererkennung, Divergenz & Artefakte

* **Woran erkennt man, dass das Programm ein falsches Ergebnis liefert oder divergiert?**
  * *Antwort:*
    * Fehler bereits im ersten Zeitschritt. \[24]
    * Divergierendes Verhalten. \[21, 24]
    * Unphysikalische Ergebnisse und Artefakte (sichtbar z. B. in Paraview), unkontrollierte Fortpflanzung von Turbulenzen. \[21, 24]
    * Verlust der Stetigkeit im Druckfeld. \[24]

