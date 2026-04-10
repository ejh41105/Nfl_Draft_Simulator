// ── Data ──
const TEAMS = [
    { abbr: 'ARI', name: 'Arizona Cardinals' },
    { abbr: 'ATL', name: 'Atlanta Falcons' },
    { abbr: 'BAL', name: 'Baltimore Ravens' },
    { abbr: 'BUF', name: 'Buffalo Bills' },
    { abbr: 'CAR', name: 'Carolina Panthers' },
    { abbr: 'CHI', name: 'Chicago Bears' },
    { abbr: 'CIN', name: 'Cincinnati Bengals' },
    { abbr: 'CLE', name: 'Cleveland Browns' },
    { abbr: 'DAL', name: 'Dallas Cowboys' },
    { abbr: 'DEN', name: 'Denver Broncos' },
    { abbr: 'DET', name: 'Detroit Lions' },
    { abbr: 'GB',  name: 'Green Bay Packers' },
    { abbr: 'HOU', name: 'Houston Texans' },
    { abbr: 'IND', name: 'Indianapolis Colts' },
    { abbr: 'JAC', name: 'Jacksonville Jaguars' },
    { abbr: 'KC',  name: 'Kansas City Chiefs' },
    { abbr: 'LV',  name: 'Las Vegas Raiders' },
    { abbr: 'LAR', name: 'Los Angeles Rams' },
    { abbr: 'LAC', name: 'Los Angeles Chargers' },
    { abbr: 'MIA', name: 'Miami Dolphins' },
    { abbr: 'MIN', name: 'Minnesota Vikings' },
    { abbr: 'NE',  name: 'New England Patriots' },
    { abbr: 'NO',  name: 'New Orleans Saints' },
    { abbr: 'NYG', name: 'New York Giants' },
    { abbr: 'NYJ', name: 'New York Jets' },
    { abbr: 'PHI', name: 'Philadelphia Eagles' },
    { abbr: 'PIT', name: 'Pittsburgh Steelers' },
    { abbr: 'SF',  name: 'San Francisco 49ers' },
    { abbr: 'SEA', name: 'Seattle Seahawks' },
    { abbr: 'TB',  name: 'Tampa Bay Buccaneers' },
    { abbr: 'TEN', name: 'Tennessee Titans' },
    { abbr: 'WAS', name: 'Washington Commanders' },
];

const YEARS  = ['26'];
const ROUNDS = [1, 2, 3, 4, 5, 6, 7];
const BASE_PICKS_PER_ROUND = 32;

// 2026 NFL Draft compensatory picks by round (rounds 3–7 only per CBA rules)
const COMP_PICKS = { 1: 0, 2: 0, 3: 4, 4: 8, 5: 9, 6: 3, 7: 9 };

function totalPicks(rounds) {
    let total = 0;
    for (let r = 1; r <= rounds; r++) {
        total += BASE_PICKS_PER_ROUND + (COMP_PICKS[r] || 0);
    }
    return total;
}

// ── State ──
let selectedTeams  = new Set();
let selectedYear   = '26';
let selectedRounds = 1;
let selectedSpeed  = 'fast';

// ── Build Team Grid ──
function buildTeams() {
    const grid = document.getElementById('teamGrid');
    TEAMS.forEach(team => {
        const btn = document.createElement('button');
        btn.className = 'team-btn';
        btn.dataset.abbr = team.abbr;
        btn.title = team.name;
        btn.innerHTML = `
      <img class="team-logo"
           src="https://a.espncdn.com/i/teamlogos/nfl/500/${team.abbr.toLowerCase()}.png"
           alt="${team.abbr}"
           onerror="this.style.display='none'" />
      ${team.abbr}
    `;
        btn.addEventListener('click', () => toggleTeam(btn, team.abbr));
        grid.appendChild(btn);
    });
}

function toggleTeam(btn, abbr) {
    if (selectedTeams.has(abbr)) {
        selectedTeams.delete(abbr);
        btn.classList.remove('selected');
    } else {
        selectedTeams.add(abbr);
        btn.classList.add('selected');
    }
    updateSummary();
}

// ── Build Year Pills ──
function buildYears() {
    const row = document.getElementById('yearRow');
    YEARS.forEach(y => {
        const pill = document.createElement('div');
        pill.className = 'pill' + (y === selectedYear ? ' selected' : '');
        pill.textContent = `'${y}`;
        pill.addEventListener('click', () => {
            document.querySelectorAll('#yearRow .pill').forEach(p => p.classList.remove('selected'));
            pill.classList.add('selected');
            selectedYear = y;
            updateSummary();
        });
        row.appendChild(pill);
    });
}

// ── Build Round Pills ──
function buildRounds() {
    const row = document.getElementById('roundsRow');
    ROUNDS.forEach(r => {
        const pill = document.createElement('div');
        pill.className = 'pill' + (r === selectedRounds ? ' selected' : '');
        pill.textContent = r;
        pill.addEventListener('click', () => {
            document.querySelectorAll('#roundsRow .pill').forEach(p => p.classList.remove('selected'));
            pill.classList.add('selected');
            selectedRounds = r;
            updateSummary();
        });
        row.appendChild(pill);
    });
}

// ── Speed ──
function selectSpeed(card) {
    document.querySelectorAll('.speed-card').forEach(c => c.classList.remove('selected'));
    card.classList.add('selected');
    selectedSpeed = card.dataset.speed;
    updateSummary();
}

// ── Summary ──
function updateSummary() {
    document.getElementById('summaryTeams').textContent  = selectedTeams.size;
    document.getElementById('summaryYear').textContent   = `'${selectedYear}`;
    document.getElementById('summaryRounds').textContent = selectedRounds;
    document.getElementById('summarySpeed').textContent  =
        selectedSpeed.charAt(0).toUpperCase() + selectedSpeed.slice(1);
    document.getElementById('summaryPicks').textContent  = totalPicks(selectedRounds);
}

// ── Start Draft ──
function startDraft() {
    const msg = document.getElementById('validationMsg');
    if (selectedTeams.size === 0) {
        msg.classList.add('visible');
        return;
    }
    msg.classList.remove('visible');

    const config = {
        teams:  Array.from(selectedTeams),
        year:   selectedYear,
        rounds: selectedRounds,
        speed:  selectedSpeed,
    };
    sessionStorage.setItem('draftConfig', JSON.stringify(config));
    window.location.href = 'DraftPage.html';
}

// ── Init ──
document.addEventListener('DOMContentLoaded', () => {
    // Team grid
    buildTeams();
    buildYears();
    buildRounds();

    // Select All / Deselect All buttons
    document.querySelector('.ctrl-btn:nth-child(1)').addEventListener('click', () => {
        document.querySelectorAll('.team-btn').forEach(btn => {
            btn.classList.add('selected');
            selectedTeams.add(btn.dataset.abbr);
        });
        updateSummary();
    });

    document.querySelector('.ctrl-btn:nth-child(2)').addEventListener('click', () => {
        document.querySelectorAll('.team-btn').forEach(btn => btn.classList.remove('selected'));
        selectedTeams.clear();
        updateSummary();
    });

    // Speed cards
    document.querySelectorAll('.speed-card').forEach(card => {
        card.addEventListener('click', () => selectSpeed(card));
    });

    // Start button
    document.getElementById('startBtn').addEventListener('click', startDraft);

    updateSummary();
});