#pragma once

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Smart Clip Dashboard</title>
<style>
  :root {
    --glass-bg: rgba(255, 255, 255, 0.28);
    --glass-border: rgba(255, 255, 255, 0.45);
    --glass-shadow: 0 12px 36px rgba(15, 23, 42, 0.12);
    --text-main: #1e293b;
    --text-muted: rgba(30, 41, 59, 0.72);
    --temp-color: #f97316;
    --hum-color: #0ea5e9;
  }

  body.state-rainy {
    --glass-bg: rgba(15, 23, 42, 0.34);
    --glass-border: rgba(255, 255, 255, 0.22);
    --glass-shadow: 0 12px 36px rgba(0, 0, 0, 0.28);
    --text-main: #f8fafc;
    --text-muted: rgba(248, 250, 252, 0.75);
    --temp-color: #fb923c;
    --hum-color: #38bdf8;
  }

  * {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
  }

  body {
    font-family: ui-rounded, 'SF Pro Rounded', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
    min-height: 100vh;
    color: var(--text-main);
    transition: color 1.2s ease;
    overflow-x: hidden;
    position: relative;
  }

  /* Dynamic Sky Layers for seamless cross-fade */
  .sky-layer {
    position: fixed;
    inset: 0;
    z-index: -3;
    opacity: 0;
    transition: opacity 1.8s ease-in-out;
  }

  .sky-sunny {
    background: linear-gradient(160deg, #60a5fa 0%, #93c5fd 42%, #fde68a 100%);
  }

  .sky-cloudy {
    background: linear-gradient(160deg, #94a3b8 0%, #cbd5e1 48%, #e2e8f0 100%);
  }

  .sky-rainy {
    background: linear-gradient(160deg, #1e293b 0%, #334155 48%, #475569 100%);
  }

  body.state-sunny .sky-sunny { opacity: 1; }
  body.state-cloudy .sky-cloudy { opacity: 1; }
  body.state-rainy .sky-rainy { opacity: 1; }

  /* Effects Layer */
  .fx-layer {
    position: fixed;
    inset: 0;
    z-index: -2;
    pointer-events: none;
    overflow: hidden;
  }

  /* Sun + Sunshine Glow */
  .sun-wrap {
    position: absolute;
    top: -40px;
    right: -40px;
    width: 320px;
    height: 320px;
    opacity: 0;
    transform: scale(0.85);
    transition: opacity 1.8s ease, transform 1.8s ease;
  }

  body.state-sunny .sun-wrap {
    opacity: 1;
    transform: scale(1);
  }

  .sun-core {
    position: absolute;
    top: 70px;
    right: 70px;
    width: 110px;
    height: 110px;
    border-radius: 50%;
    background: radial-gradient(circle, #fffbeb 0%, #fde047 55%, #f59e0b 100%);
    box-shadow: 0 0 60px rgba(253, 224, 71, 0.85), 0 0 120px rgba(251, 191, 36, 0.5);
    animation: sunPulse 5s ease-in-out infinite;
  }

  .sun-rays {
    position: absolute;
    top: 15px;
    right: 15px;
    width: 220px;
    height: 220px;
    border-radius: 50%;
    background: repeating-conic-gradient(
      from 0deg,
      rgba(255, 251, 235, 0.28) 0deg 12deg,
      transparent 12deg 30deg
    );
    mask-image: radial-gradient(circle, rgba(0,0,0,0.9) 25%, transparent 72%);
    -webkit-mask-image: radial-gradient(circle, rgba(0,0,0,0.9) 25%, transparent 72%);
    animation: spinSlow 40s linear infinite;
  }

  @keyframes sunPulse {
    0%, 100% { transform: scale(1); }
    50% { transform: scale(1.06); }
  }

  @keyframes spinSlow {
    from { transform: rotate(0deg); }
    to { transform: rotate(360deg); }
  }

  /* Cute Drifting Clouds */
  .clouds-wrap {
    position: absolute;
    inset: 0;
    opacity: 0;
    transition: opacity 1.8s ease;
  }

  body.state-cloudy .clouds-wrap,
  body.state-rainy .clouds-wrap {
    opacity: 1;
  }

  body.state-sunny .clouds-wrap {
    opacity: 0.35;
  }

  .cloud {
    position: absolute;
    background: rgba(255, 255, 255, 0.65);
    border-radius: 999px;
    filter: blur(2px);
    animation: drift linear infinite;
  }

  body.state-rainy .cloud {
    background: rgba(148, 163, 184, 0.32);
  }

  .cloud::before, .cloud::after {
    content: '';
    position: absolute;
    background: inherit;
    border-radius: 50%;
  }

  .c1 { width: 160px; height: 48px; top: 10%; left: -180px; animation-duration: 38s; }
  .c1::before { width: 70px; height: 70px; top: -30px; left: 26px; }
  .c1::after { width: 52px; height: 52px; top: -20px; left: 82px; }

  .c2 { width: 210px; height: 58px; top: 24%; left: -240px; animation-duration: 52s; animation-delay: -18s; }
  .c2::before { width: 88px; height: 88px; top: -38px; left: 36px; }
  .c2::after { width: 64px; height: 64px; top: -24px; left: 108px; }

  .c3 { width: 130px; height: 40px; top: 68%; left: -150px; animation-duration: 44s; animation-delay: -9s; }
  .c3::before { width: 56px; height: 56px; top: -24px; left: 22px; }
  .c3::after { width: 42px; height: 42px; top: -16px; left: 66px; }

  @keyframes drift {
    from { transform: translateX(0); }
    to { transform: translateX(calc(100vw + 300px)); }
  }

  /* Rain Canvas */
  #rainCanvas {
    position: fixed;
    inset: 0;
    z-index: -1;
    pointer-events: none;
    opacity: 0;
    transition: opacity 1.6s ease;
  }

  body.state-rainy #rainCanvas {
    opacity: 1;
  }

  /* Main Layout */
  .container {
    max-width: 820px;
    margin: 0 auto;
    padding: 28px 18px 40px;
    display: flex;
    flex-direction: column;
    gap: 18px;
  }

  .header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 4px 6px;
  }

  .title {
    font-size: 1.45rem;
    font-weight: 700;
    letter-spacing: -0.02em;
    display: flex;
    align-items: center;
    gap: 10px;
  }

  .live-pill {
    display: inline-flex;
    align-items: center;
    gap: 7px;
    font-size: 0.78rem;
    font-weight: 600;
    padding: 6px 12px;
    border-radius: 999px;
    background: var(--glass-bg);
    border: 1px solid var(--glass-border);
    backdrop-filter: blur(14px);
    -webkit-backdrop-filter: blur(14px);
    color: var(--text-muted);
  }

  .dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: #22c55e;
    box-shadow: 0 0 8px #22c55e;
    animation: blink 2.2s ease-in-out infinite;
  }

  .dot.offline {
    background: #ef4444;
    box-shadow: 0 0 8px #ef4444;
    animation: none;
  }

  @keyframes blink {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.4; }
  }

  /* Frosted Glass Cards */
  .card {
    background: var(--glass-bg);
    border: 1px solid var(--glass-border);
    border-radius: 24px;
    padding: 22px 24px;
    backdrop-filter: blur(20px);
    -webkit-backdrop-filter: blur(20px);
    box-shadow: var(--glass-shadow);
    transition: background 1.2s ease, border-color 1.2s ease, box-shadow 1.2s ease;
  }

  /* Top-Level Rain Status Hero */
  .hero {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 16px;
    flex-wrap: wrap;
  }

  .hero-left {
    display: flex;
    align-items: center;
    gap: 16px;
  }

  .weather-badge {
    width: 62px;
    height: 62px;
    border-radius: 20px;
    background: rgba(255, 255, 255, 0.35);
    display: grid;
    place-items: center;
    font-size: 2rem;
    box-shadow: inset 0 1px 1px rgba(255, 255, 255, 0.6);
  }

  body.state-rainy .weather-badge {
    background: rgba(255, 255, 255, 0.12);
  }

  .label {
    font-size: 0.78rem;
    text-transform: uppercase;
    letter-spacing: 0.08em;
    font-weight: 700;
    color: var(--text-muted);
    margin-bottom: 4px;
  }

  .rain-status {
    font-size: 1.85rem;
    font-weight: 800;
    letter-spacing: -0.02em;
    line-height: 1.1;
  }

  .hero-stats {
    display: flex;
    gap: 20px;
  }

  .mini-stat {
    text-align: right;
  }

  .mini-val {
    font-size: 1.55rem;
    font-weight: 800;
    letter-spacing: -0.02em;
  }

  /* Charts Grid */
  .charts-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 18px;
  }

  @media (max-width: 680px) {
    .charts-grid {
      grid-template-columns: 1fr;
    }
    .hero-stats {
      width: 100%;
      justify-content: space-between;
    }
    .mini-stat {
      text-align: left;
    }
  }

  .chart-head {
    display: flex;
    justify-content: space-between;
    align-items: baseline;
    margin-bottom: 12px;
  }

  .chart-val {
    font-size: 1.5rem;
    font-weight: 800;
  }

  .chart-range {
    font-size: 0.78rem;
    color: var(--text-muted);
    font-weight: 600;
  }

  .chart-box {
    width: 100%;
    height: 150px;
    position: relative;
  }

  svg.spark {
    width: 100%;
    height: 100%;
    overflow: visible;
  }
</style>
</head>
<body class="state-sunny">
  <div class="sky-layer sky-sunny"></div>
  <div class="sky-layer sky-cloudy"></div>
  <div class="sky-layer sky-rainy"></div>

  <div class="fx-layer">
    <div class="sun-wrap">
      <div class="sun-rays"></div>
      <div class="sun-core"></div>
    </div>
    <div class="clouds-wrap">
      <div class="cloud c1"></div>
      <div class="cloud c2"></div>
      <div class="cloud c3"></div>
    </div>
  </div>

  <canvas id="rainCanvas"></canvas>

  <main class="container">
    <header class="header">
      <div class="title">Smart Clip</div>
      <div class="live-pill">
        <span id="liveDot" class="dot"></span>
        <span id="liveText">Live · 5s</span>
      </div>
    </header>

    <section class="card hero">
      <div class="hero-left">
        <div id="weatherIcon" class="weather-badge">☀️</div>
        <div>
          <div class="label">Rain Status</div>
          <div id="rainStatus" class="rain-status">Not Raining</div>
        </div>
      </div>
      <div class="hero-stats">
        <div class="mini-stat">
          <div class="label">Temp</div>
          <div id="heroTemp" class="mini-val">--°C</div>
        </div>
        <div class="mini-stat">
          <div class="label">Humidity</div>
          <div id="heroHum" class="mini-val">--%</div>
        </div>
      </div>
    </section>

    <section class="charts-grid">
      <div class="card">
        <div class="chart-head">
          <div>
            <div class="label">Temperature</div>
            <div id="tempVal" class="chart-val">--°C</div>
          </div>
          <div id="tempRange" class="chart-range">-- / --</div>
        </div>
        <div class="chart-box">
          <svg id="tempChart" class="spark" viewBox="0 0 320 140" preserveAspectRatio="none"></svg>
        </div>
      </div>

      <div class="card">
        <div class="chart-head">
          <div>
            <div class="label">Humidity</div>
            <div id="humVal" class="chart-val">--%</div>
          </div>
          <div id="humRange" class="chart-range">-- / --</div>
        </div>
        <div class="chart-box">
          <svg id="humChart" class="spark" viewBox="0 0 320 140" preserveAspectRatio="none"></svg>
        </div>
      </div>
    </section>
  </main>

<script>
  // Subtle Rain Animation on Canvas
  const canvas = document.getElementById('rainCanvas');
  const ctx = canvas.getContext('2d');
  let drops = [];
  let isRainyMode = false;

  function resizeCanvas() {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    drops = Array.from({ length: 65 }, () => ({
      x: Math.random() * canvas.width,
      y: Math.random() * canvas.height,
      len: 12 + Math.random() * 14,
      speed: 5 + Math.random() * 6,
      alpha: 0.18 + Math.random() * 0.28
    }));
  }

  window.addEventListener('resize', resizeCanvas);
  resizeCanvas();

  function animateRain() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    if (isRainyMode) {
      ctx.lineWidth = 1.4;
      ctx.lineCap = 'round';
      for (const d of drops) {
        ctx.strokeStyle = `rgba(186, 230, 253, ${d.alpha})`;
        ctx.beginPath();
        ctx.moveTo(d.x, d.y);
        ctx.lineTo(d.x - 2, d.y + d.len);
        ctx.stroke();
        d.y += d.speed;
        d.x -= 0.7;
        if (d.y > canvas.height) {
          d.y = -d.len;
          d.x = Math.random() * canvas.width;
        }
      }
    }
    requestAnimationFrame(animateRain);
  }
  animateRain();

  function drawChart(svgId, values, color, gradId) {
    const svg = document.getElementById(svgId);
    const valid = values.filter(v => v !== null && !isNaN(v));
    if (valid.length === 0) {
      svg.innerHTML = '';
      return;
    }

    const w = 320, h = 140, padY = 16;
    let min = Math.min(...valid);
    let max = Math.max(...valid);
    if (max - min < 1) {
      min -= 0.5;
      max += 0.5;
    }

    const pts = valid.map((v, i) => {
      const x = valid.length === 1 ? w / 2 : (i / (valid.length - 1)) * w;
      const y = h - padY - ((v - min) / (max - min)) * (h - padY * 2);
      return [x, y];
    });

    const lineD = pts.map((p, i) => `${i === 0 ? 'M' : 'L'}${p[0].toFixed(1)},${p[1].toFixed(1)}`).join(' ');
    const areaD = `${lineD} L${pts[pts.length - 1][0].toFixed(1)},${h} L${pts[0][0].toFixed(1)},${h} Z`;
    const last = pts[pts.length - 1];

    svg.innerHTML = `
      <defs>
        <linearGradient id="${gradId}" x1="0" y1="0" x2="0" y2="1">
          <stop offset="0%" stop-color="${color}" stop-opacity="0.38"/>
          <stop offset="100%" stop-color="${color}" stop-opacity="0.02"/>
        </linearGradient>
      </defs>
      <path d="${areaD}" fill="url(#${gradId})"/>
      <path d="${lineD}" fill="none" stroke="${color}" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"/>
      <circle cx="${last[0].toFixed(1)}" cy="${last[1].toFixed(1)}" r="4.5" fill="${color}" stroke="#fff" stroke-width="2"/>
    `;
  }

  function updateWeatherState(raining, temp, hum) {
    const body = document.body;
    const icon = document.getElementById('weatherIcon');
    const status = document.getElementById('rainStatus');

    body.classList.remove('state-sunny', 'state-cloudy', 'state-rainy');

    if (raining) {
      body.classList.add('state-rainy');
      isRainyMode = true;
      icon.textContent = '🌧️';
      status.textContent = 'Raining';
    } else if ((hum !== null && hum >= 75) || (temp !== null && temp < 22)) {
      body.classList.add('state-cloudy');
      isRainyMode = false;
      icon.textContent = '⛅';
      status.textContent = 'Not Raining';
    } else {
      body.classList.add('state-sunny');
      isRainyMode = false;
      icon.textContent = '☀️';
      status.textContent = 'Not Raining';
    }
  }

  async function poll() {
    const dot = document.getElementById('liveDot');
    const liveText = document.getElementById('liveText');
    try {
      const res = await fetch('/api/data', { cache: 'no-store' });
      if (!res.ok) throw new Error('HTTP ' + res.status);
      const data = await res.json();

      dot.classList.remove('offline');
      liveText.textContent = 'Live · 5s';

      const cur = data.current;
      const tStr = cur.temp !== null ? `${cur.temp.toFixed(1)}°C` : '--°C';
      const hStr = cur.hum !== null ? `${cur.hum.toFixed(1)}%` : '--%';

      document.getElementById('heroTemp').textContent = tStr;
      document.getElementById('heroHum').textContent = hStr;
      document.getElementById('tempVal').textContent = tStr;
      document.getElementById('humVal').textContent = hStr;

      updateWeatherState(cur.raining, cur.temp, cur.hum);

      const temps = data.history.map(p => p.temp).filter(v => v !== null);
      const hums = data.history.map(p => p.hum).filter(v => v !== null);

      if (temps.length) {
        document.getElementById('tempRange').textContent = `${Math.min(...temps).toFixed(1)}° – ${Math.max(...temps).toFixed(1)}°`;
      }
      if (hums.length) {
        document.getElementById('humRange').textContent = `${Math.min(...hums).toFixed(0)}% – ${Math.max(...hums).toFixed(0)}%`;
      }

      const cs = getComputedStyle(document.body);
      drawChart('tempChart', data.history.map(p => p.temp), cs.getPropertyValue('--temp-color').trim(), 'tempGrad');
      drawChart('humChart', data.history.map(p => p.hum), cs.getPropertyValue('--hum-color').trim(), 'humGrad');
    } catch (e) {
      dot.classList.add('offline');
      liveText.textContent = 'Reconnecting…';
    }
  }

  poll();
  setInterval(poll, 5000);
</script>
</body>
</html>
)rawliteral";
