// ── DATA ──
// Both arrays start empty and are populated by init() via fetch().
// To switch from local files to your Crow API, just change the paths:
//   'players.json' → '/api/players'
//   'picks.json'   → '/api/picks'
let PLAYERS     = [];
let DRAFT_ORDER = [];

// O(1) player lookup by playerId.
// Built after fetch completes in init(). Use: PLAYER_MAP.get(playerId)
let PLAYER_MAP = new Map();

// Players currently drafted, keyed by playerId
const drafted = new Set();

// Which overall pick is currently on the clock (1-indexed)
let currentPick = 1;

// Completed pick results keyed by overall pick number
// Format: { [overall]: { playerName, position, college } }
const pickResults = {};

// ── HELPERS ──

function posBadgeClass(pos) {
  const map = {
    QB:'QB', WR:'WR', RB:'RB', TE:'TE',
    OT:'OT', OL:'OL', OG:'OG', OC:'OC',
    EDGE:'EDGE', DL:'DL', ILB:'ILB', OLB:'OLB',
    LB:'LB', CB:'CB', S:'S', K:'K', P:'P'
  };
  return map[pos] || '';
}

function formatHeight(totalInches) {
  const feet   = Math.floor(totalInches / 12);
  const inches = totalInches % 12;
  return `${feet}\u2032${inches}\u2033`;
}

// ── PLAYER CARD BUILDER ──
// Generates the full inner HTML of the modal from a single player object.
// When you add a new field to your C++ Player class and JSON, add it here.
function buildPlayerCard(p) {
  const rasClass = p.RAS >= 8 ? 'ras-high' : p.RAS >= 5 ? 'ras-mid' : 'ras-low';
  const stats    = p.majorStats || '—';

  return `
    <button class="modal-close" onclick="closePlayerCard()">
      <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
        <line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/>
      </svg>
    </button>

    <div class="modal-top">
      <div class="modal-badge-row">
        <span class="pos-badge ${posBadgeClass(p.position)}">${p.position}</span>
        <span class="card-number">#${p.number}</span>
      </div>

      <div class="modal-name-row">
        <span class="card-name">${p.name}</span>
        <div class="card-logo-circle" title="${p.college}"></div>
      </div>

      <span class="card-college">${p.college}</span>

      <div class="modal-vitals">
        <div class="vital-box">
          <span class="vital-label">Age</span>
          <span class="vital-value">${p.age}</span>
        </div>
        <div class="vital-box">
          <span class="vital-label">Height</span>
          <span class="vital-value">${formatHeight(p.height)}</span>
        </div>
        <div class="vital-box">
          <span class="vital-label">Weight</span>
          <span class="vital-value">${p.weight} lb</span>
        </div>
      </div>
    </div>

    <div class="modal-bottom">
      <div class="vital-box">
        <span class="vital-label">Overall rank</span>
        <span class="vital-value">#${p.consensusRanking}</span>
      </div>
      <div class="vital-box">
        <span class="vital-label">Pos. rank</span>
        <span class="vital-value">#${p.positionalRanking}</span>
      </div>
      <div class="vital-box">
        <span class="vital-label">RAS</span>
        <span class="vital-value ${rasClass}">${p.RAS.toFixed(2)}</span>
      </div>
    </div>

    <div class="modal-stats">
      <span class="vital-label">Major stats</span>
      <p class="stats-text">${stats}</p>
    </div>
  `;
}

// ── MODAL: OPEN / CLOSE ──
function openPlayerCard(p) {
  document.getElementById('modal-card-inner').innerHTML = buildPlayerCard(p);
  document.getElementById('player-modal').classList.add('open');
}

function closePlayerCard() {
  document.getElementById('player-modal').classList.remove('open');
}

document.getElementById('player-modal').addEventListener('click', (e) => {
  if (e.target === document.getElementById('player-modal')) closePlayerCard();
});

document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape') closePlayerCard();
});

// ── PLAYER ROW BUILDER ──
// Builds one row for the right-panel player pool list.
function renderPlayerRow(p) {
  const row = document.createElement('div');
  row.className = 'player-row';
  row.innerHTML = `
    <span class="player-rank">${p.consensusRanking}</span>
    <div class="player-logo-circle"></div>
    <div class="player-row-info">
      <div class="player-row-name">${p.name}</div>
      <div class="player-row-school">${p.position} · ${p.college}</div>
    </div>
    <span class="pos-badge ${posBadgeClass(p.position)}">${p.position}</span>
    <span class="player-consensus-rank">#${p.consensusRanking}</span>
  `;
  row.addEventListener('click', () => openPlayerCard(p));
  return row;
}

// ── PLAYER POOL RENDERER ──
// Clears and rebuilds the right panel based on current filter + search.
function renderPlayers(filter = 'ALL', search = '') {
  const body = document.getElementById('player-body');
  body.innerHTML = '';
  let count = 0;
  const term = search.toLowerCase();

  PLAYERS.forEach(p => {
    if (drafted.has(p.playerId)) return;
    if (filter !== 'ALL' && p.position !== filter) return;
    if (term &&
        !p.name.toLowerCase().includes(term) &&
        !p.college.toLowerCase().includes(term)) return;

    count++;
    body.appendChild(renderPlayerRow(p));
  });

  document.getElementById('avail-count').textContent = count;
}

// ── PICK ROW BUILDER ──
// Builds one pick row for the draft board.
// state: 'empty' | 'on-clock' | 'completed'
function buildPickRow(pick, state, result = null) {
  const row = document.createElement('div');
  row.className = `pick-row ${state}`;
  row.id = `pick-row-${pick.overall}`;

  let playerName, playerMeta, badge;

  if (state === 'completed' && result) {
    playerName = result.playerName;
    playerMeta = `${result.position} · ${result.college}`;
    badge      = `<span class="pos-badge ${posBadgeClass(result.position)}">${result.position}</span>`;
  } else if (state === 'on-clock') {
    playerName = 'On the clock…';
    playerMeta = pick.teamId;
    badge      = `<span class="pos-badge">—</span>`;
  } else {
    playerName = '—';
    playerMeta = pick.teamId;
    badge      = `<span class="pos-badge"></span>`;
  }

  row.innerHTML = `
    <span class="pick-num">${pick.overall}</span>
    <div class="team-logo">${pick.teamId}</div>
    <div class="pick-info">
      <div class="player-name">${playerName}</div>
      <div class="player-meta">${playerMeta}</div>
    </div>
    ${badge}
  `;

  return row;
}

// ── DRAFT BOARD RENDERER ──
// Reads DRAFT_ORDER and builds the entire left panel.
// Groups picks by round with sticky round-label headers.
function renderDraftBoard() {
  const body = document.getElementById('draft-body');
  body.innerHTML = '';

  let currentRound = 0;

  DRAFT_ORDER.forEach(pick => {
    if (pick.round !== currentRound) {
      const roundPicks = DRAFT_ORDER.filter(p => p.round === pick.round);
      const first      = roundPicks[0].overall;
      const last       = roundPicks[roundPicks.length - 1].overall;

      const label = document.createElement('div');
      label.className = 'round-label';
      label.innerHTML = `<span>ROUND ${pick.round}</span><span>Picks ${first}–${last}</span>`;
      body.appendChild(label);

      currentRound = pick.round;
    }

    let state;
    if (pick.overall < currentPick)      state = 'completed';
    else if (pick.overall === currentPick) state = 'on-clock';
    else                                   state = 'empty';

    body.appendChild(buildPickRow(pick, state, pickResults[pick.overall] || null));
  });

  const total = DRAFT_ORDER.length;
  document.getElementById('total-picks').textContent  = total;
  document.getElementById('current-pick').textContent = currentPick;
  document.getElementById('draft-total').textContent  = `${total} TOTAL PICKS`;

  const onClockRow = document.getElementById(`pick-row-${currentPick}`);
  if (onClockRow) onClockRow.scrollIntoView({ block: 'center', behavior: 'smooth' });
}

// ── RECORD A PICK ──
// Call this when your Crow backend confirms a pick:
//   recordPick(7, "David Bailey", "EDGE", "Texas Tech")
// It surgically updates only the two affected rows instead of re-rendering the whole board.
function recordPick(overall, playerName, position, college) {
  pickResults[overall] = { playerName, position, college };

  const player = PLAYERS.find(p => p.name === playerName);
  if (player) {
    drafted.add(player.playerId);
    renderPlayers(activeFilter, document.getElementById('player-search').value);
  }

  currentPick = overall + 1;

  const completedRow = document.getElementById(`pick-row-${overall}`);
  if (completedRow) {
    const pick   = DRAFT_ORDER.find(p => p.overall === overall);
    const newRow = buildPickRow(pick, 'completed', pickResults[overall]);
    newRow.id    = completedRow.id;
    completedRow.replaceWith(newRow);
  }

  const nextPick = DRAFT_ORDER.find(p => p.overall === currentPick);
  if (nextPick) {
    const nextRow = document.getElementById(`pick-row-${currentPick}`);
    if (nextRow) {
      const newNextRow = buildPickRow(nextPick, 'on-clock');
      newNextRow.id    = nextRow.id;
      nextRow.replaceWith(newNextRow);
      newNextRow.scrollIntoView({ block: 'center', behavior: 'smooth' });
    }
  }

  document.getElementById('current-pick').textContent = currentPick;

  const done  = currentPick - 1;
  const total = DRAFT_ORDER.length;
  document.getElementById('progress-fill').style.width = `${(done / total * 100).toFixed(1)}%`;
  document.getElementById('prog-done').textContent = `${done} done`;
  document.getElementById('prog-left').textContent = `${total - done} left`;
}

// ── FILTER TABS ──
let activeFilter = 'ALL';

function setFilter(el, f) {
  document.querySelectorAll('.filter-tab').forEach(t => t.classList.remove('active'));
  el.classList.add('active');
  activeFilter = f;
  renderPlayers(f, document.getElementById('player-search').value);
}

// ── SEARCH ──
document.getElementById('player-search').addEventListener('input', e => {
  renderPlayers(activeFilter, e.target.value);
});

// ── SPEED BUTTONS ──
function setSpeed(el, s) {
  document.querySelectorAll('.speed-btn').forEach(b => b.classList.remove('active'));
  el.classList.add('active');
}

// ── PAUSE / RESUME ──
let paused = false;

document.getElementById('btn-pause').addEventListener('click', () => {
  paused = !paused;
  const btn = document.getElementById('btn-pause');
  if (paused) {
    btn.innerHTML = `<svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor"><polygon points="5 3 19 12 5 21 5 3"/></svg> Resume Draft`;
    btn.classList.remove('primary');
    btn.style.borderColor = 'var(--green)';
    btn.style.color = 'var(--green)';
  } else {
    btn.innerHTML = `<svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor"><rect x="6" y="4" width="4" height="16"/><rect x="14" y="4" width="4" height="16"/></svg> Pause Draft`;
    btn.classList.add('primary');
    btn.style.borderColor = '';
    btn.style.color = '';
  }
});

// ── RESTART ──
document.getElementById('btn-restart').addEventListener('click', () => {
  if (!confirm('Restart the entire draft? All picks will be cleared.')) return;
  // TODO: fetch('/api/draft/restart', { method: 'POST' })
  alert('Draft restarted! (Hook up your Crow backend here)');
});

// ── INIT ──
// Fetches both JSON files in parallel, populates the data arrays, then renders.
// Switch file paths to API endpoints when Crow is ready:
//   'players.json' → '/api/players'
//   'picks.json'   → '/api/picks'
async function init() {
  try {
    const [players, picks] = await Promise.all([
      fetch('../../BackEnd/Data/JSONS/Players.json').then(r => {
        if (!r.ok) throw new Error(`Players.json: ${r.status}`);
        return r.json();
      }),
      fetch('../../BackEnd/Data/JSONS/DraftOrder.json').then(r => {
        if (!r.ok) throw new Error(`DraftOrder.json: ${r.status}`);
        return r.json();
      })
    ]);

    PLAYERS     = players;
    DRAFT_ORDER = picks;
    PLAYER_MAP  = new Map(players.map(p => [p.playerId, p]));

    renderDraftBoard();
    renderPlayers();

  } catch (err) {
    console.error('Failed to load draft data:', err);
    document.getElementById('draft-body').innerHTML =
      `<p style="padding:1rem;color:var(--danger);font-size:0.8rem;">
        Could not load data: ${err.message}<br>
        Make sure you are serving files over HTTP (not file://).
      </p>`;
  }
}

init();