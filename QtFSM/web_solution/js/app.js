// app.js — theme, toast, keyboard shortcuts, state editor modal, init
//
// This is the entry point that wires everything together.
// The initDemoFsm() function sets up a demo FSM.
//
// Key functions:
//   toggleTheme()      — switch dark/light
//   toast(msg,type)    — show a notification
//   openSE(id)         — open state editor modal for a state
//   saveSE()           — save state editor changes back to model
//   initDemoFsm()      — load demo FSM and wire up events

/* ═══ THEME ═══ */
var dark = true;

function toggleTheme() {
    dark = !dark;
    document.documentElement.setAttribute('data-theme', dark ? 'dark' : 'light');

    /* Update all theme toggle buttons (homepage + workspace) */
    var icons = ['ti', 'hti'];
    var labels = ['tl', 'htl'];
    icons.forEach(function(id) {
        var el = document.getElementById(id);
        if (el) { el.textContent = dark ? '☀' : '🌙'; }
    });
    labels.forEach(function(id) {
        var el = document.getElementById(id);
        if (el) { el.textContent = dark ? 'Light' : 'Dark'; }
    });

    if (typeof renderAll === 'function') { renderAll(); }
}

/* ═══ TOAST ═══ */
var toastTimeout = null;

function toast(msg, type) {
    type = type || 'ok';
    var t = document.getElementById('toast');
    if (!t) { return; }
    t.textContent = msg;
    t.className = 'show t' + type;
    clearTimeout(toastTimeout);
    toastTimeout = setTimeout(function() {
        t.className = '';
    }, 2400);
}

/* ═══ KEYBOARD ═══ */
document.addEventListener('keydown', function(e) {
    /* Ignore when typing in inputs */
    var tg = document.activeElement ? document.activeElement.tagName : '';
    if (tg === 'INPUT' || tg === 'TEXTAREA' || tg === 'SELECT') { return; }

    /* Ignore when on homepage */
    var homepage = document.getElementById('homepage');
    if (homepage && homepage.style.display !== 'none') { return; }

    if (e.ctrlKey && e.key === 'z') {
        e.preventDefault();
        undo();
    }
    if (e.ctrlKey && (e.key === 'y' || e.key === 'Y')) {
        e.preventDefault();
        redo();
    }
    if (e.key === 'Delete' && selId) {
        snap();
        fsm.states = fsm.states.filter(function(s) { return s.id !== selId; });
        fsm.transitions = fsm.transitions.filter(function(t) {
            return t.id !== selId && t.from !== selId && t.to !== selId;
        });
        selId = null;
        renderAll();
    }
    if (e.key === 'Escape') {
        conFrom = null;
        setMode('sel');
        closeModal();
        closeCV();
    }
    if (e.key === 'f' || e.key === 'F') { fitView(); }
    if (e.key === 's' || e.key === 'S') { setMode('sel'); }
    if (e.key === 'c' || e.key === 'C') { setMode('con'); }
    if (e.key === 'd' || e.key === 'D') { setMode('del'); }
    if (e.key === '=' || e.key === '+') { zoomIn(); }
    if (e.key === '-') { zoomOut(); }
    if (e.key === 'v' || e.key === 'V') { toggleCV(); }
    if (e.key === 'x' || e.key === 'X') { toggleCanvasMax(); }
});

/* ═══ STATE EDITOR MODAL ═══ */
var seId = null;

function openSE(id) {
    var s = fsm.states.find(function(x) { return x.id === id; });
    if (!s) { return; }
    seId = id;

    /* dot color */
    var dot = document.getElementById('se-dot');
    if (dot) {
        dot.style.background = s.type === 'initial' ? 'var(--green)' :
            s.type === 'final' ? 'var(--red)' : 'var(--accent)';
    }
    var title = document.getElementById('se-title');
    if (title) { title.textContent = 'Edit: ' + s.name; }

    /* populate General */
    var nameEl = document.getElementById('se-name');
    if (nameEl) { nameEl.value = s.name || ''; }
    var typeEl = document.getElementById('se-type');
    if (typeEl) { typeEl.value = s.type || 'normal'; }
    var histEl = document.getElementById('se-history');
    if (histEl) { histEl.value = s.history || 'none'; }
    var commentEl = document.getElementById('se-comment');
    if (commentEl) { commentEl.value = s.comment || ''; }

    /* populate Entry */
    var entryEl = document.getElementById('se-onentry');
    if (entryEl) { entryEl.value = s.onEntry || ''; }
    var entryGuardEl = document.getElementById('se-entry-guard');
    if (entryGuardEl) { entryGuardEl.value = s.entryGuard || ''; }

    /* populate Exit */
    var exitEl = document.getElementById('se-onexit');
    if (exitEl) { exitEl.value = s.onExit || ''; }
    var exitGuardEl = document.getElementById('se-exit-guard');
    if (exitGuardEl) { exitGuardEl.value = s.exitGuard || ''; }

    /* populate Internal Events */
    renderIERows(s.internalEvents || []);

    /* reset to General tab */
    setSETab('general');
    var overlay = document.getElementById('se-overlay');
    if (overlay) { overlay.classList.add('open'); }
    setTimeout(function() {
        if (nameEl) { nameEl.focus(); }
    }, 60);
}

function closeSE() {
    var overlay = document.getElementById('se-overlay');
    if (overlay) { overlay.classList.remove('open'); }
    seId = null;
}

function setSETab(tab) {
    ['general', 'entry', 'exit', 'internal'].forEach(function(t) {
        var tabEl = document.getElementById('set-' + t);
        if (tabEl) { tabEl.classList.toggle('active', t === tab); }
        var contentEl = document.getElementById('se-' + t);
        if (contentEl) { contentEl.style.display = t === tab ? 'block' : 'none'; }
    });
}

function renderIERows(events) {
    var tbody = document.getElementById('ie-tbody');
    if (!tbody) { return; }
    tbody.innerHTML = '';
    (events || []).forEach(function(ev, i) {
        var tr = document.createElement('tr');
        tr.innerHTML =
            '<td><input class="ie-input" placeholder="EVENT_NAME" value="' +
            (ev.event || '') + '" oninput="updateIE(' + i + ',\'event\',this.value)"></td>' +
            '<td><input class="ie-input" placeholder="guard" value="' +
            (ev.guard || '') + '" oninput="updateIE(' + i + ',\'guard\',this.value)"></td>' +
            '<td><input class="ie-input" placeholder="action()" value="' +
            (ev.action || '') + '" oninput="updateIE(' + i + ',\'action\',this.value)"></td>' +
            '<td><button class="ie-del" onclick="removeIE(' + i + ')" title="Remove">✕</button></td>';
        tbody.appendChild(tr);
    });
}

function updateIE(idx, field, val) {
    var s = fsm.states.find(function(x) { return x.id === seId; });
    if (s && s.internalEvents[idx]) {
        s.internalEvents[idx][field] = val;
    }
}

function addIERow() {
    var s = fsm.states.find(function(x) { return x.id === seId; });
    if (!s) { return; }
    s.internalEvents = s.internalEvents || [];
    s.internalEvents.push({ event: '', guard: '', action: '' });
    renderIERows(s.internalEvents);
    setTimeout(function() {
        var rows = document.querySelectorAll('#ie-tbody tr');
        if (rows.length > 0) {
            var lastInput = rows[rows.length - 1].querySelector('.ie-input');
            if (lastInput) { lastInput.focus(); }
        }
    }, 30);
}

function removeIE(idx) {
    var s = fsm.states.find(function(x) { return x.id === seId; });
    if (!s) { return; }
    s.internalEvents.splice(idx, 1);
    renderIERows(s.internalEvents);
}

function saveSE() {
    var s = fsm.states.find(function(x) { return x.id === seId; });
    if (!s) { return; }
    snap();

    var nameEl = document.getElementById('se-name');
    var newName = nameEl ? nameEl.value.trim() : s.name;
    if (!newName) { newName = s.name; }
    s.name = newName;

    var typeEl = document.getElementById('se-type');
    if (typeEl) { s.type = typeEl.value; }
    var histEl = document.getElementById('se-history');
    if (histEl) { s.history = histEl.value; }
    var commentEl = document.getElementById('se-comment');
    if (commentEl) { s.comment = commentEl.value.trim(); }

    var entryEl = document.getElementById('se-onentry');
    if (entryEl) { s.onEntry = entryEl.value.trim(); }
    var entryGuardEl = document.getElementById('se-entry-guard');
    if (entryGuardEl) { s.entryGuard = entryGuardEl.value.trim(); }

    var exitEl = document.getElementById('se-onexit');
    if (exitEl) { s.onExit = exitEl.value.trim(); }
    var exitGuardEl = document.getElementById('se-exit-guard');
    if (exitGuardEl) { s.exitGuard = exitGuardEl.value.trim(); }

    /* internalEvents already updated live via updateIE() */
    if (s.type === 'initial') { fsm.initial = s.id; }
    renderAll();
    closeSE();
    toast('State "' + newName + '" saved ✓', 'ok');
}

/* close on backdrop click */
if (typeof document !== 'undefined') {
    var seOverlay = document.getElementById('se-overlay');
    if (seOverlay) {
        seOverlay.addEventListener('click', function(e) {
            if (e.target === seOverlay) { closeSE(); }
        });
    }
}

/* ═══ DEMO FSM INIT ═══ */

function initDemoFsm() {
    var s1 = nid();
    var s2 = nid();
    var s3 = nid();
    var s4 = nid();
    fsm.states = [
        { id: s1, name: 'Idle', type: 'initial', x: 160, y: 210,
          onEntry: '', onExit: '', entryGuard: '', exitGuard: '',
          internalEvents: [], comment: '', history: 'none' },
        { id: s2, name: 'Running', type: 'normal', x: 390, y: 110,
          onEntry: 'startTimer();', onExit: 'stopTimer();',
          entryGuard: '', exitGuard: '',
          internalEvents: [{ event: 'TICK', guard: '', action: 'updateProgress()' }],
          comment: 'Main running state', history: 'none' },
        { id: s3, name: 'Paused', type: 'normal', x: 390, y: 310,
          onEntry: 'pauseTimer();', onExit: '', entryGuard: '', exitGuard: '',
          internalEvents: [], comment: '', history: 'none' },
        { id: s4, name: 'Stopped', type: 'final', x: 620, y: 210,
          onEntry: 'onStop();', onExit: '', entryGuard: '', exitGuard: '',
          internalEvents: [], comment: 'Terminal state', history: 'none' }
    ];
    fsm.initial = s1;
    fsm.transitions = [
        { id: nid(), from: s1, to: s2, event: 'START', guard: '', action: 'onStart()' },
        { id: nid(), from: s2, to: s3, event: 'PAUSE', guard: '', action: 'onPause()' },
        { id: nid(), from: s3, to: s2, event: 'RESUME', guard: '', action: 'onResume()' },
        { id: nid(), from: s2, to: s4, event: 'STOP', guard: '', action: 'onStop()' },
        { id: nid(), from: s3, to: s4, event: 'STOP', guard: '', action: 'onStop()' },
        { id: nid(), from: s4, to: s1, event: 'RESET', guard: '', action: 'onReset()' }
    ];

    var codeTa = document.getElementById('code-ta');
    if (codeTa) {
        codeTa.value =
            '// ── DSL Format Example ──────────────────────────────\n' +
            '// Paste FSM code and click "Parse & Draw"\n' +
            '// ─────────────────────────────────────────────────────\n' +
            'FSM: MyFSM\n' +
            '\n' +
            'STATE: Idle\n' +
            'STATE: Running\n' +
            'STATE: Paused\n' +
            'FINAL: Stopped\n' +
            'INIT: Idle\n' +
            '\n' +
            'ON_ENTRY: Running / startTimer(); setLed(ON)\n' +
            'ON_EXIT:  Running / stopTimer()\n' +
            'ON_ENTRY: Stopped / logDone()\n' +
            '\n' +
            'INTERNAL: Running / TICK [] / updateProgress()\n' +
            '\n' +
            'TRANS: Idle    -> Running : START  [] / onStart()\n' +
            'TRANS: Running -> Paused  : PAUSE  [] / onPause()\n' +
            'TRANS: Paused  -> Running : RESUME [] / onResume()\n' +
            'TRANS: Running -> Stopped : STOP   [] / onStop()\n' +
            'TRANS: Paused  -> Stopped : STOP   [] / onStop()\n' +
            'TRANS: Stopped -> Idle    : RESET  [] / onReset()';
    }
}

/* ═══ PAGE INIT — start on homepage ═══ */
/* No auto-init: user clicks a card to enter workspace */
