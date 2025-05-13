import streamlit as st
import re
import time
from collections import deque

import numpy as np
import plotly.express as px
import pandas as pd


LOG_FILE = "/home/italo/esp/projetos/sde/esp_log.txt"
ANSI_RE  = re.compile(r"\x1b\[[0-9;]*m")
DATA_RE  = re.compile(r"Angle:\s*(\d+)°?.*?Dist(?:ance)?:\s*(-?[\d.]+)", flags=re.IGNORECASE)
MAX_POINTS = 100
REFRESH_INTERVAL = 0.5  






def parse(line: str):
    clean = ANSI_RE.sub("", line)
    m = DATA_RE.search(clean)
    if m:
        ang = int(m.group(1))
        dist = float(m.group(2))
        if dist >= 0:
            return ang, dist
    return None




## configura o streamlit
# carrega histórico e buffers
def main():
    st.set_page_config(page_title="Radar Polar Realtime", layout="wide")
    st.title("🛰️ Radar Polar em Tempo Real")

    
    with st.sidebar:
        st.header("Configurações")
        max_pts = st.slider("Máx. de Pontos no Buffer", 10, 500, MAX_POINTS)
        interval = st.slider("Intervalo (s)", 0.1, 2.0, REFRESH_INTERVAL, 0.1)






















    
    thetas = deque(maxlen=max_pts)
    rs     = deque(maxlen=max_pts)

    
    with open(LOG_FILE, "r", encoding="utf-8", errors="ignore") as fp:
        for line in fp:
            p = parse(line)
            if p:
                ang, dist = p
                thetas.append(ang)
                rs.append(dist)

    fp = open(LOG_FILE, "r", encoding="utf-8", errors="ignore")
    fp.seek(0, 2)

    placeholder = st.empty()

    while True:
        new_line = fp.readline()
        if new_line:
            p = parse(new_line)
            if p:
                ang, dist = p
                thetas.append(ang)
                rs.append(dist)






        
        df = pd.DataFrame({
            "angle": list(thetas),
            "distance": list(rs)
        })

        






        fig = px.scatter_polar(
            df,
            theta="angle",
            r="distance",
            title="Posições detectadas",
            template="plotly_dark",
            hover_data={"angle":True, "distance":True},
        )
        fig.update_layout(
            polar=dict(
                angularaxis=dict(direction="clockwise", rotation=180, dtick=30),
                radialaxis=dict(range=[0, max(rs or [1])*1.1])
            ),
            margin=dict(l=20, r=20, t=50, b=20)
        )

        placeholder.plotly_chart(fig, use_container_width=True)
        time.sleep(interval)





if __name__ == "__main__":
    main()
