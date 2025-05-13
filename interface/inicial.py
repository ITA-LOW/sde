"""
Lê todo o esp_log.txt, plota o que já existe e depois atualiza em tempo real.
Ignora qualquer coisa que não seja linha de leitura (Angle / Dist ou Distance).
"""

import re
import time
from collections import deque
from pathlib import Path
import matplotlib.pyplot as plt

LOG_PATH = Path("/home/italo/esp/projetos/sde/esp_log.txt")
ANSI_RE  = re.compile(r"\x1b\[[0-9;]*m")           # remove sequências ANSI
DATA_RE  = re.compile(
    r"Angle:\s*(\d+)°?.*?Dist(?:ance)?:\s*(-?[\d.]+)",  # aceita Dist: ou Distance:
    flags=re.IGNORECASE
)

x_vals, y_vals = deque(maxlen=360), deque(maxlen=360)

def parse(raw_line: str):
    """Devolve (angle, dist) ou None."""
    clean = ANSI_RE.sub("", raw_line)              # tira códigos de cor
    m = DATA_RE.search(clean)
    if m:
        ang, dist = int(m.group(1)), float(m.group(2))
        if dist >= 0:                              # ignora time-out (-1.00 cm)
            return ang, dist
    return None

# ---------- carrega histórico ----------
with LOG_PATH.open("r", encoding="utf-8", errors="ignore") as fp:
    for line in fp:
        parsed = parse(line)
        if parsed:
            x_vals.append(parsed[0])
            y_vals.append(parsed[1])

    # ---------- configura o gráfico ----------
    plt.ion()
    fig, ax = plt.subplots()
    line, = ax.plot(x_vals, y_vals, lw=2)
    ax.set_xlabel("Ângulo (°)")
    ax.set_ylabel("Distância (cm)")
    ax.set_xlim(0, 180)
    ax.set_ylim(0, (max(y_vals) if y_vals else 1) * 1.1)
    fig.canvas.draw(); fig.canvas.flush_events()

    # ---------- segue “tailing” ----------
    fp.seek(0, 2)                                  # garante ponteiro no fim
    while True:
        txt = fp.readline()
        if not txt:
            time.sleep(0.05)
            continue

        if parsed := parse(txt):
            ang, dist = parsed
            x_vals.append(ang); y_vals.append(dist)

            line.set_data(x_vals, y_vals)
            ax.set_ylim(0, max(y_vals) * 1.1)
            fig.canvas.draw(); fig.canvas.flush_events()
