// ==============================
// DraftPage.js
// Backend-driven draft page
// ==============================

// ---------- App State ----------
let PLAYERS = [];
let DRAFT_ORDER = [];
let PLAYER_MAP = new Map();

let currentPick = 1;
let currentIsUserPick = false;
let activeFilter = 'ALL';
let paused = false;
let draftStarted = false;
let isAdvancing = false;
let autoAdvanceTimer = null;
let autoAdvanceDelayResolve = null;
let postUserPickDelayMs = 0;
let selectedSpeed = 'base';
let recommendedRanks = new Set();
let mobileActiveTab = 'draft';

const pickResults = {}; // overall -> { playerId, playerName, position, college }

// ---------- Helpers ----------
function posBadgeClass(pos) {
  const map = {
    QB: 'QB', WR: 'WR', RB: 'RB', TE: 'TE',
    OT: 'OT', OL: 'OL', OG: 'OG', OC: 'OC',
    EDGE: 'EDGE', DL: 'DL', ILB: 'ILB', OLB: 'OLB',
    LB: 'LB', CB: 'CB', S: 'S', K: 'K', P: 'P'
  };
  return map[pos] || '';
}

function formatHeight(totalInches) {
  if (!totalInches && totalInches !== 0) return '—';
  const feet = Math.floor(totalInches / 12);
  const inches = totalInches % 12;
  return `${feet}\u2032${inches}\u2033`;
}

function escapeHtml(value) {
  return String(value)
    .replaceAll('&', '&amp;')
    .replaceAll('<', '&lt;')
    .replaceAll('>', '&gt;')
    .replaceAll('"', '&quot;')
    .replaceAll("'", '&#39;');
}

function getSavedConfig() {
  const raw = sessionStorage.getItem('draftConfig');
  if (!raw) {
    throw new Error('No draft config found. Go back to the homepage and start a draft.');
  }

  const config = JSON.parse(raw);

  return {
    teams: Array.isArray(config.teams) ? config.teams : [],
    year: config.year || '26',
    rounds: Number(config.rounds || 1),
    speed: normalizeSpeedValue(config.speed)
  };
}

function getSearchValue() {
  const el = document.getElementById('player-search');
  return el ? el.value : '';
}

function getTeamLogoUrl(teamId) {
  return `https://a.espncdn.com/i/teamlogos/nfl/500/${String(teamId || '').toLowerCase()}.png`;
}

function normalizeSpeedValue(speed) {
  const map = {
    slow: 'slow',
    fast: 'fast',
    base: 'base',
    '1x': 'base',
    '0.5x': 'slow',
    '2x': 'fast',
    '5x': 'fast'
  };

  return map[speed] || 'base';
}

function getCpuPickDelayMs(speed) {
  switch (normalizeSpeedValue(speed)) {
    case 'slow':
      return 2000;
    case 'fast':
      return 0;
    case 'base':
    default:
      return 500;
  }
}

function syncSpeedButtons() {
  document.querySelectorAll('.speed-btn').forEach(button => {
    button.classList.toggle('active', button.dataset.speed === selectedSpeed);
  });
}

function setDraftCompleteUI(isComplete) {
  const completeCard = document.getElementById('draft-complete-card');
  const completeModal = document.getElementById('draft-complete-modal');
  const pauseBtn = document.getElementById('btn-pause');
  const restartBtn = document.getElementById('btn-restart');
  const speedGroup = document.querySelector('.speed-group');

  if (completeCard) {
    completeCard.classList.toggle('open', isComplete);
  }

  if (completeModal) {
    completeModal.classList.toggle('open', isComplete);
  }

  if (pauseBtn) {
    pauseBtn.disabled = isComplete;
    pauseBtn.style.display = isComplete ? 'none' : '';
  }

  if (restartBtn) {
    restartBtn.style.display = isComplete ? 'none' : '';
  }

  if (speedGroup) {
    speedGroup.style.display = isComplete ? 'none' : '';
  }
}

function cancelAutoAdvanceDelay() {
  if (autoAdvanceTimer) {
    clearTimeout(autoAdvanceTimer);
    autoAdvanceTimer = null;
  }

  if (autoAdvanceDelayResolve) {
    const resolve = autoAdvanceDelayResolve;
    autoAdvanceDelayResolve = null;
    resolve();
  }
}

function setMobileTab(view) {
  mobileActiveTab = view === 'players' ? 'players' : 'draft';

  const main = document.getElementById('draft-page');
  if (main) {
    main.classList.toggle('mobile-tab-draft', mobileActiveTab === 'draft');
    main.classList.toggle('mobile-tab-players', mobileActiveTab === 'players');
  }

  const draftTab = document.getElementById('mobile-tab-draft');
  const playersTab = document.getElementById('mobile-tab-players');

  if (draftTab) {
    draftTab.classList.toggle('active', mobileActiveTab === 'draft');
  }

  if (playersTab) {
    playersTab.classList.toggle('active', mobileActiveTab === 'players');
  }
}

// ---------- Modal ----------
function buildPlayerCard(p) {
  const rasValue = typeof p.RAS === 'number' ? p.RAS : null;
  const rasClass =
    rasValue === null ? '' :
    rasValue >= 8 ? 'ras-high' :
    rasValue >= 5 ? 'ras-mid' : 'ras-low';

  const stats = p.majorStats || '—';
  const overallRank = p.consensusRanking ?? p.consensusRank ?? '—';
  const posRank = p.positionalRanking ?? '—';
  const age = p.age ?? '—';
  const height = p.height != null ? formatHeight(p.height) : '—';
  const weight = p.weight != null ? `${p.weight} lb` : '—';
  const jersey = p.number ?? '—';

  const consensusRank = p.consensusRanking ?? p.consensusRank ?? null;
  const canDraftPlayer = currentIsUserPick && consensusRank != null;

  return `
    <button class="modal-close" onclick="closePlayerCard()">
      <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
        <line x1="18" y1="6" x2="6" y2="18"></line>
        <line x1="6" y1="6" x2="18" y2="18"></line>
      </svg>
    </button>

    <div class="modal-top">
      <div class="modal-badge-row">
        <span class="pos-badge ${posBadgeClass(p.position)}">${escapeHtml(p.position || '—')}</span>
        <span class="card-number">#${escapeHtml(jersey)}</span>
      </div>

      <div class="modal-name-row">
        <span class="card-name">${escapeHtml(p.name || 'Unknown Player')}</span>
        <div class="card-logo-circle" title="${escapeHtml(p.college || '')}"></div>
      </div>

      <span class="card-college">${escapeHtml(p.college || '—')}</span>

      <div class="modal-vitals">
        <div class="vital-box">
          <span class="vital-label">Age</span>
          <span class="vital-value">${escapeHtml(age)}</span>
        </div>
        <div class="vital-box">
          <span class="vital-label">Height</span>
          <span class="vital-value">${escapeHtml(height)}</span>
        </div>
        <div class="vital-box">
          <span class="vital-label">Weight</span>
          <span class="vital-value">${escapeHtml(weight)}</span>
        </div>
      </div>
    </div>

    <div class="modal-bottom">
      <div class="vital-box">
        <span class="vital-label">Overall rank</span>
        <span class="vital-value">#${escapeHtml(overallRank)}</span>
      </div>
      <div class="vital-box">
        <span class="vital-label">Pos. rank</span>
        <span class="vital-value">#${escapeHtml(posRank)}</span>
      </div>
      <div class="vital-box">
        <span class="vital-label">RAS</span>
        <span class="vital-value ${rasClass}">
          ${rasValue === null ? '—' : escapeHtml(rasValue.toFixed(2))}
        </span>
      </div>
    </div>

    <div class="modal-stats">
      <span class="vital-label">Major stats</span>
      <p class="stats-text">${escapeHtml(stats)}</p>
    </div>

    ${canDraftPlayer ? `
      <div class="modal-actions">
        <button class="ctrl-btn primary modal-draft-btn" onclick="draftPlayerFromModal(${Number(consensusRank)})">
          Draft Player
        </button>
      </div>
    ` : ''}
  `;
}

function openPlayerCard(p) {
  const modalInner = document.getElementById('modal-card-inner');
  const modal = document.getElementById('player-modal');
  if (!modalInner || !modal) return;

  modalInner.innerHTML = buildPlayerCard(p);
  modal.classList.add('open');
}

function closePlayerCard() {
  const modal = document.getElementById('player-modal');
  if (!modal) return;
  modal.classList.remove('open');
}

// ---------- Player Pool ----------
function renderPlayerRow(p, isUserOnClock) {
  const row = document.createElement('div');
  row.className = 'player-row';

  const consensusRank = p.consensusRanking ?? p.consensusRank ?? '—';
  const isRecommended = isUserOnClock && recommendedRanks.has(Number(consensusRank));

  row.innerHTML = `
    <span class="player-rank">${escapeHtml(consensusRank)}</span>
    <div class="player-logo-circle"></div>
    <div class="player-row-info">
      <div class="player-row-name">${escapeHtml(p.name || 'Unknown Player')}</div>
      <div class="player-row-school">${escapeHtml((p.position || '—') + ' · ' + (p.college || '—'))}</div>
    </div>
    ${isRecommended ? '<span class="recommend-chip">Recommended Player</span>' : ''}
    <span class="pos-badge ${posBadgeClass(p.position)}">${escapeHtml(p.position || '—')}</span>
    <span class="player-consensus-rank">#${escapeHtml(consensusRank)}</span>
  `;

  row.addEventListener('click', () => {
    openPlayerCard(p);
  });

  return row;
}

function renderPlayers(filter = 'ALL', search = '', isUserOnClock = false) {
  const body = document.getElementById('player-body');
  if (!body) return;

  body.innerHTML = '';
  const term = search.trim().toLowerCase();

  const filtered = PLAYERS.filter(p => {
    if (filter !== 'ALL' && p.position !== filter) return false;

    if (term) {
      const name = (p.name || '').toLowerCase();
      const college = (p.college || '').toLowerCase();
      if (!name.includes(term) && !college.includes(term)) return false;
    }

    return true;
  });

  filtered.forEach(p => body.appendChild(renderPlayerRow(p, isUserOnClock)));

  const countEl = document.getElementById('avail-count');
  if (countEl) countEl.textContent = filtered.length;
}

// ---------- Draft Board ----------
function buildPickRow(pick, state, result = null) {
  const row = document.createElement('div');
  row.className = `pick-row ${state}`;
  row.id = `pick-row-${pick.overall}`;

  let playerName = '—';
  let playerMeta = pick.teamId;
  let badge = `<span class="pos-badge"></span>`;

  if (state === 'completed' && result) {
    playerName = result.playerName;
    playerMeta = `${result.position} · ${result.college}`;
    badge = `<span class="pos-badge ${posBadgeClass(result.position)}">${escapeHtml(result.position)}</span>`;
  } else if (state === 'on-clock') {
    playerName = 'On the clock…';
    playerMeta = pick.teamId;
    badge = `<span class="pos-badge">—</span>`;
  }

  row.innerHTML = `
    <span class="pick-num">${escapeHtml(pick.overall)}</span>
    <div class="team-logo">
      <img
        src="${escapeHtml(getTeamLogoUrl(pick.teamId))}"
        alt="${escapeHtml(pick.teamId)}"
        title="${escapeHtml(pick.teamId)}"
        onerror="this.style.display='none'; this.parentElement.textContent='${escapeHtml(pick.teamId)}';"
      />
    </div>
    <div class="pick-info">
      <div class="player-name">${escapeHtml(playerName)}</div>
      <div class="player-meta">${escapeHtml(playerMeta)}</div>
    </div>
    ${badge}
  `;

  return row;
}

function scrollDraftBoardToCurrentPick() {
  const onClockRow = document.getElementById(`pick-row-${currentPick}`);
  const draftBody = document.getElementById('draft-body');
  if (!onClockRow || !draftBody) return;

  const rowTop = onClockRow.offsetTop;
  const rowBottom = rowTop + onClockRow.offsetHeight;
  const viewTop = draftBody.scrollTop;
  const viewBottom = viewTop + draftBody.clientHeight;
  const isFastCpuRun = !currentIsUserPick && normalizeSpeedValue(selectedSpeed) === 'fast';

  if (isFastCpuRun) {
    if (rowTop < viewTop) {
      draftBody.scrollTo({
        top: rowTop,
        behavior: 'auto'
      });
    } else if (rowBottom > viewBottom) {
      draftBody.scrollTo({
        top: Math.max(rowBottom - draftBody.clientHeight, 0),
        behavior: 'auto'
      });
    }
    return;
  }

  const targetTop = rowTop - ((draftBody.clientHeight - onClockRow.offsetHeight) / 2);
  draftBody.scrollTo({
    top: Math.max(targetTop, 0),
    behavior: currentIsUserPick ? 'smooth' : 'auto'
  });
}

function renderDraftBoard() {
  const body = document.getElementById('draft-body');
  if (!body) return;

  body.innerHTML = '';

  let currentRound = 0;

  DRAFT_ORDER.forEach(pick => {
    if (pick.round !== currentRound) {
      const roundPicks = DRAFT_ORDER.filter(p => p.round === pick.round);
      const first = roundPicks[0].overall;
      const last = roundPicks[roundPicks.length - 1].overall;

      const label = document.createElement('div');
      label.className = 'round-label';
      label.innerHTML = `<span>ROUND ${pick.round}</span><span>Picks ${first}–${last}</span>`;
      body.appendChild(label);

      currentRound = pick.round;
    }

    let state;
    if (pick.overall < currentPick) state = 'completed';
    else if (pick.overall === currentPick) state = 'on-clock';
    else state = 'empty';

    body.appendChild(buildPickRow(pick, state, pickResults[pick.overall] || null));
  });

  const total = DRAFT_ORDER.length;
  const totalEl = document.getElementById('total-picks');
  const currentEl = document.getElementById('current-pick');
  const totalLabel = document.getElementById('draft-total');

  if (totalEl) totalEl.textContent = total;
  if (currentEl) currentEl.textContent = currentPick;
  if (totalLabel) totalLabel.textContent = `${total} TOTAL PICKS`;
  scrollDraftBoardToCurrentPick();
}

// ---------- UI Updates ----------
function updateProgress(done, total) {
  const safeTotal = Math.max(total, 1);
  const fill = document.getElementById('progress-fill');
  const doneEl = document.getElementById('prog-done');
  const leftEl = document.getElementById('prog-left');

  if (fill) fill.style.width = `${((done / safeTotal) * 100).toFixed(1)}%`;
  if (doneEl) doneEl.textContent = `${done} done`;
  if (leftEl) leftEl.textContent = `${Math.max(total - done, 0)} left`;
}

function updateClockDisplay(isUserPick) {
  const display = document.getElementById('clock-display');
  if (!display) return;

  if (paused) display.textContent = 'PAUSE';
  else if (isUserPick) display.textContent = 'USER';
  else display.textContent = 'AUTO';
}

function setOnClockBox(teamName, overall, round) {
  const teamEl = document.querySelector('.on-clock-team');
  const pickEl = document.querySelector('.on-clock-pick');

  if (teamEl) teamEl.textContent = teamName || '—';
  if (pickEl) pickEl.textContent = `Pick #${overall || 0} · Round ${round || 0}`;
}

// ---------- Backend ----------
async function startBackendDraft() {
  const config = getSavedConfig();

  selectedSpeed = normalizeSpeedValue(config.speed);
  syncSpeedButtons();

  const res = await fetch('/api/draft/start', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(config)
  });

  if (!res.ok) {
    const text = await res.text();
    throw new Error(text || 'Failed to start backend draft.');
  }

  draftStarted = true;
}

async function fetchDraftState() {
  const res = await fetch('/api/draft/state');
  if (!res.ok) {
    const text = await res.text();
    throw new Error(text || 'Failed to load draft state.');
  }
  return res.json();
}

async function fetchPlayers() {
  const params = new URLSearchParams();
  if (activeFilter && activeFilter !== 'ALL') params.set('position', activeFilter);

  const search = getSearchValue().trim();
  if (search) params.set('search', search);

  const url = params.toString() ? `/api/players?${params.toString()}` : '/api/players';

  const res = await fetch(url);
  if (!res.ok) {
    const text = await res.text();
    throw new Error(text || 'Failed to load players.');
  }
  return res.json();
}

async function fetchPicks() {
  const res = await fetch('/api/picks');
  if (!res.ok) {
    const text = await res.text();
    throw new Error(text || 'Failed to load picks.');
  }
  return res.json();
}

async function advanceCpuPick() {
  const res = await fetch('/api/draft/advance', { method: 'POST' });
  if (!res.ok) {
    const text = await res.text();
    throw new Error(text || 'Failed to advance draft.');
  }
  return res.json();
}

async function submitUserPick(consensusRank) {
  const res = await fetch('/api/draft/pick', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({ consensusRank })
  });

  if (!res.ok) {
    const text = await res.text();
    throw new Error(text || 'Failed to submit pick.');
  }

  const data = await res.json();
  if (!data.ok) {
    throw new Error('Backend rejected pick.');
  }

  postUserPickDelayMs = 900;
  syncState(data.state);
}

async function draftPlayerFromModal(consensusRank) {
  try {
    await submitUserPick(consensusRank);
    closePlayerCard();
  } catch (err) {
    console.error(err);
    alert(err.message);
  }
}

async function restartDraft() {
  cancelAutoAdvanceDelay();
  paused = false;
  setDraftCompleteUI(false);

  const pauseBtn = document.getElementById('btn-pause');
  if (pauseBtn) {
    pauseBtn.innerHTML = `
      <svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor">
        <rect x="6" y="4" width="4" height="16"></rect>
        <rect x="14" y="4" width="4" height="16"></rect>
      </svg>
      Pause Draft
    `;
    pauseBtn.classList.add('primary');
    pauseBtn.style.borderColor = '';
    pauseBtn.style.color = '';
  }

  for (const key of Object.keys(pickResults)) {
    delete pickResults[key];
  }

  PLAYERS = [];
  PLAYER_MAP = new Map();
  currentPick = 1;

  await startBackendDraft();
  await refreshFromBackend();
}

// ---------- State Sync ----------
function inferCompletedResultsFromBoard(state) {
  for (const key of Object.keys(pickResults)) {
    delete pickResults[key];
  }

  const results = Array.isArray(state.results) ? state.results : [];

  results.forEach(result => {
    if (!result || typeof result.overall !== 'number' || !result.player) return;

    pickResults[result.overall] = {
      playerId: result.player.id,
      playerName: result.player.name,
      position: result.player.position,
      college: result.player.college
    };
  });
}

function rebuildPlayerMap() {
  PLAYER_MAP = new Map(
    PLAYERS.map(p => [
      p.playerId ?? p.id ?? p.consensusRanking ?? p.consensusRank,
      p
    ])
  );
}

function removeDraftedPlayersFromPool(state) {
  const draftedIds = new Set();
  const draftedRanks = new Set();
  const results = Array.isArray(state.results) ? state.results : [];

  results.forEach(result => {
    if (!result || !result.player) return;

    if (result.player.id != null) {
      draftedIds.add(String(result.player.id));
    }

    if (result.player.consensusRank != null) {
      draftedRanks.add(Number(result.player.consensusRank));
    }
  });

  PLAYERS = PLAYERS.filter(player => {
    const playerId = player.playerId ?? player.id;
    const consensusRank = player.consensusRanking ?? player.consensusRank;

    if (playerId != null && draftedIds.has(String(playerId))) {
      return false;
    }

    if (consensusRank != null && draftedRanks.has(Number(consensusRank))) {
      return false;
    }

    return true;
  });
}

function syncState(state) {
  if (!state.started && draftStarted) {
    throw new Error('Draft session disappeared.');
  }

  currentPick = state.overall || 1;
  currentIsUserPick = Boolean(state.isUserPick);
  recommendedRanks = new Set(
    Array.isArray(state.recommendedConsensusRanks)
      ? state.recommendedConsensusRanks.map(rank => Number(rank))
      : []
  );

  inferCompletedResultsFromBoard(state);
  removeDraftedPlayersFromPool(state);
  rebuildPlayerMap();

  renderDraftBoard();
  renderPlayers(activeFilter, getSearchValue(), currentIsUserPick);

  setOnClockBox(state.onClockTeamName, state.overall, state.round);
  updateProgress(
    Array.isArray(state.results) ? state.results.length : Math.max((state.currentPickNumber || 1) - 1, 0),
    DRAFT_ORDER.length
  );
  updateClockDisplay(state.isUserPick);

  const currentEl = document.getElementById('current-pick');
  if (currentEl) currentEl.textContent = state.overall || 1;

  const availEl = document.getElementById('avail-count');
  if (availEl) availEl.textContent = state.availableCount ?? PLAYERS.length;

  setDraftCompleteUI(Boolean(state.complete));

  continueCpuDraft(state).catch(console.error);
}

async function refreshFromBackend() {
  if (DRAFT_ORDER.length === 0) {
    DRAFT_ORDER = await fetchPicks();
  }

  if (PLAYERS.length === 0) {
    const players = await fetchPlayers();
    PLAYERS = Array.isArray(players.players) ? players.players : players;
    rebuildPlayerMap();
  }

  const state = await fetchDraftState();
  syncState(state);
}

async function continueCpuDraft(state) {
  cancelAutoAdvanceDelay();

  if (paused || isAdvancing || state.complete || state.isUserPick) return;

  isAdvancing = true;

  try {
    let nextState = state;
    let safety = 0;

    if (postUserPickDelayMs > 0) {
      const delayMs = postUserPickDelayMs;
      postUserPickDelayMs = 0;

      await new Promise(resolve => {
        autoAdvanceDelayResolve = resolve;
        autoAdvanceTimer = setTimeout(() => {
          autoAdvanceTimer = null;
          autoAdvanceDelayResolve = null;
          resolve();
        }, delayMs);
      });

      if (paused) {
        return;
      }
    }

    while (!paused && !nextState.complete && !nextState.isUserPick) {
      const data = await advanceCpuPick();
      nextState = data.state;

      currentPick = nextState.overall || 1;
      currentIsUserPick = Boolean(nextState.isUserPick);

      inferCompletedResultsFromBoard(nextState);
      removeDraftedPlayersFromPool(nextState);
      rebuildPlayerMap();

      renderDraftBoard();
      renderPlayers(activeFilter, getSearchValue(), currentIsUserPick);
      setOnClockBox(nextState.onClockTeamName, nextState.overall, nextState.round);
      updateProgress(
        Array.isArray(nextState.results) ? nextState.results.length : Math.max((nextState.currentPickNumber || 1) - 1, 0),
        DRAFT_ORDER.length
      );
      updateClockDisplay(nextState.isUserPick);
      setDraftCompleteUI(Boolean(nextState.complete));

      const availEl = document.getElementById('avail-count');
      if (availEl) availEl.textContent = nextState.availableCount ?? PLAYERS.length;

      const delayMs = getCpuPickDelayMs(selectedSpeed);
      if (delayMs > 0 && !nextState.complete && !nextState.isUserPick && !paused) {
        await new Promise(resolve => {
          autoAdvanceDelayResolve = resolve;
          autoAdvanceTimer = setTimeout(() => {
            autoAdvanceTimer = null;
            autoAdvanceDelayResolve = null;
            resolve();
          }, delayMs);
        });
      }

      safety += 1;
      if (safety % 8 === 0) {
        await new Promise(resolve => setTimeout(resolve, 0));
      }
    }
  } finally {
    isAdvancing = false;
  }
}

// ---------- Filters / Search / Speed ----------
function setFilter(el, filterValue) {
  document.querySelectorAll('.filter-tab').forEach(t => t.classList.remove('active'));
  el.classList.add('active');
  activeFilter = filterValue;
  renderPlayers(activeFilter, getSearchValue(), currentIsUserPick);
}

function setSpeed(el, speedValue) {
  selectedSpeed = normalizeSpeedValue(speedValue);
  syncSpeedButtons();
  const config = getSavedConfig();
  config.speed = selectedSpeed;
  sessionStorage.setItem('draftConfig', JSON.stringify(config));
  cancelAutoAdvanceDelay();
}

// ---------- Event Wiring ----------
function wireEvents() {
  setMobileTab(mobileActiveTab);

  const modal = document.getElementById('player-modal');
  if (modal) {
    modal.addEventListener('click', e => {
      if (e.target === modal) closePlayerCard();
    });
  }

  document.addEventListener('keydown', e => {
    if (e.key === 'Escape') closePlayerCard();
  });

  const searchInput = document.getElementById('player-search');
  if (searchInput) {
    searchInput.addEventListener('input', () => {
      renderPlayers(activeFilter, getSearchValue(), currentIsUserPick);
    });
  }

  document.querySelectorAll('.mobile-view-tab').forEach(button => {
    button.addEventListener('click', () => {
      setMobileTab(button.dataset.view);
    });
  });

  const pauseBtn = document.getElementById('btn-pause');
  if (pauseBtn) {
    pauseBtn.addEventListener('click', async () => {
      paused = !paused;

      if (paused) {
        pauseBtn.innerHTML = `
          <svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor">
            <polygon points="5 3 19 12 5 21 5 3"></polygon>
          </svg>
          Resume Draft
        `;
        pauseBtn.classList.remove('primary');
        pauseBtn.style.borderColor = 'var(--green)';
        pauseBtn.style.color = 'var(--green)';
      } else {
        pauseBtn.innerHTML = `
          <svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor">
            <rect x="6" y="4" width="4" height="16"></rect>
            <rect x="14" y="4" width="4" height="16"></rect>
          </svg>
          Pause Draft
        `;
        pauseBtn.classList.add('primary');
        pauseBtn.style.borderColor = '';
        pauseBtn.style.color = '';
      }

      try {
        await refreshFromBackend();
      } catch (err) {
        console.error(err);
      }
    });
  }

  const restartBtn = document.getElementById('btn-restart');
  if (restartBtn) {
    restartBtn.addEventListener('click', async () => {
      const ok = window.confirm('Restart the entire draft? All picks will be cleared.');
      if (!ok) return;

      try {
        await restartDraft();
      } catch (err) {
        console.error(err);
        alert(err.message);
      }
    });
  }

  const completeRestartBtn = document.getElementById('btn-complete-restart');
  if (completeRestartBtn) {
    completeRestartBtn.addEventListener('click', async () => {
      try {
        await restartDraft();
      } catch (err) {
        console.error(err);
        alert(err.message);
      }
    });
  }
}

// ---------- Init ----------
async function init() {
  try {
    wireEvents();
    await startBackendDraft();
    await refreshFromBackend();
  } catch (err) {
    console.error('Failed to initialize draft page:', err);
    const draftBody = document.getElementById('draft-body');
    if (draftBody) {
      draftBody.innerHTML = `
        <p style="padding:1rem;color:var(--danger);font-size:0.8rem;">
          Could not initialize draft: ${escapeHtml(err.message)}
        </p>
      `;
    }
  }
}

document.addEventListener('DOMContentLoaded', init);

// Expose functions used by inline onclick in HTML
window.setFilter = setFilter;
window.setSpeed = setSpeed;
window.closePlayerCard = closePlayerCard;
window.openPlayerCard = openPlayerCard;
window.draftPlayerFromModal = draftPlayerFromModal;
