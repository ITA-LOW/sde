import re
import time
from collections import deque

import numpy as np
import matplotlib.pyplot as plt


LOG_FILE = "/home/italo/esp/projetos/sde/esp_log.txt"


ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
DATA_RE = re.compile(
    r"Angle:\s*(\d+)°?.*?Dist(?:ance)?:\s*(-?[\d.]+)",
    flags=re.IGNORECASE
)



thetas = deque(maxlen=100)
rs     = deque(maxlen=100)

def parse(line: str):
    clean = ANSI_RE.sub("", line)
    m = DATA_RE.search(clean)
    if m:
        ang = int(m.group(1))
        dist = float(m.group(2))
        if dist >= 0:
            return ang, dist
    return None

"carrefa os dados antugos"
with open(LOG_FILE, "r", encoding="utf-8", errors="ignore") as fp:
    for line in fp:
        parsed = parse(line)
        if parsed:
            ang, dist = parsed
            thetas.append(np.deg2rad(ang))
            rs.append(dist)



    
    plt.ion()
    fig, ax = plt.subplots(subplot_kw={"projection": "polar"})
    #line, = ax.plot(thetas, rs, lw=2)
    sc = ax.scatter(thetas, rs)
    ax.set_theta_zero_location("W")     
    ax.set_theta_direction(-1)          
    ax.set_rmax(max(rs) * 1.1 if rs else 1)

    fig.canvas.draw()
    fig.canvas.flush_events()

    
    fp.seek(0, 2)
    while True:
        new = fp.readline()
        if not new:
            time.sleep(0.05)
            continue

        parsed = parse(new)
        if parsed:
            ang, dist = parsed
            thetas.append(np.deg2rad(ang))
            rs.append(dist)

            #line.set_data(thetas, rs)
            sc.set_offsets(np.column_stack((thetas, rs)))
            ax.set_rmax(max(rs) * 1.1)
            fig.canvas.draw()
            fig.canvas.flush_events()
