// ui.js — code viewer, modals, property panel, tabs, navigation
//
// Everything that drives the panel UI — not the canvas.
//
// Key functions:
//   showCV(tab)         — show code viewer on given tab
//   copyCV()            — copy current tab to clipboard
//   openModal(...)      — generic confirm dialog
//   openAddState()      — Add State button
//   openAddTransition() — Add Transition button
//   refreshProps()      — update right panel from model
//   renderSL()          — render states list tab
//   renderTL()          — render transitions list tab
//   navigateToWorkspace — show workspace, hide homepage
//   navigateToHome      — show homepage, hide workspace
//   toggleCodeInput     — expand/collapse code input
//   showStateProperties — auto-show state props on click

/* ═══ NAVIGATION ═══ */

function navigateToWorkspace(type) {
    if (type !== 'fsm') {
        if (typeof toast === 'function') {
            toast('Coming soon!', 'err');
        }
        return;
    }
    var homepage = document.getElementById('homepage');
    var workspace = document.getElementById('workspace-page');
    if (homepage) { homepage.style.display = 'none'; }
    if (workspace) { workspace.style.display = 'block'; }
    /* Initialize demo FSM if empty */
    if (fsm.states.length === 0) {
        initDemoFsm();
    }
    setTimeout(function() { renderAll(); }, 50);
}

function navigateToHome() {
    var homepage = document.getElementById('homepage');
    var workspace = document.getElementById('workspace-page');
    if (homepage) { homepage.style.display = 'flex'; }
    if (workspace) { workspace.style.display = 'none'; }
}

/* ═══ CODE INPUT EXPAND/COLLAPSE ═══ */

var codeInputExpanded = true;

function toggleCodeInput() {
    codeInputExpanded = !codeInputExpanded;
    var wrapper = document.getElementById('code-input-wrapper');
    var btn = document.getElementById('ced-toggle-btn');
    if (wrapper) {
        if (codeInputExpanded) {
            wrapper.classList.remove('code-input-collapsed');
            wrapper.classList.add('code-input-expanded');
        } else {
            wrapper.classList.remove('code-input-expanded');
            wrapper.classList.add('code-input-collapsed');
        }
    }
    if (btn) {
        btn.textContent = codeInputExpanded ? '▼' : '▶';
    }
}

/* ═══ CODE VIEWER ═══ */
var cvOpen = false;
var cvXP = false;
var cvTab = 'cpp';
var cvData = { cpp: '', json: '' };

function toggleCV() {
    cvOpen = !cvOpen;
    var cv = document.getElementById('cv');
    var btn = document.getElementById('btn-cv');
    var rh = document.getElementById('cv-resize');
    if (cv) {
        cv.classList.toggle('open', cvOpen);
        if (!cvOpen) { cv.classList.remove('xp'); cvXP = false; }
    }
    if (btn) { btn.classList.toggle('active', cvOpen); }
    if (rh) { rh.classList.toggle('active', cvOpen); }
}

function closeCV() {
    cvOpen = false;
    cvXP = false;
    var cv = document.getElementById('cv');
    if (cv) { cv.className = ''; }
    var btn = document.getElementById('btn-cv');
    if (btn) { btn.classList.remove('active'); }
    var rh = document.getElementById('cv-resize');
    if (rh) { rh.classList.remove('active'); }
}

function toggleCVExpand() {
    cvXP = !cvXP;
    var cv = document.getElementById('cv');
    if (cv) {
        cv.classList.toggle('xp', cvXP);
        if (cvXP) { cv.classList.add('open'); }
    }
    var xbtn = document.getElementById('cv-xbtn');
    if (xbtn) { xbtn.textContent = cvXP ? '⤡' : '⤢'; }
    var cc = document.getElementById('cc');
    if (cc) { cc.classList.toggle('cve', cvXP && cvOpen); }
}

function setCVTab(t) {
    cvTab = t;
    ['cpp', 'json'].forEach(function(x) {
        var el = document.getElementById('cvt-' + x);
        if (el) { el.classList.toggle('active', x === t); }
    });
    renderCV();
}

function renderCV() {
    var body = document.getElementById('cv-body');
    if (!body) { return; }
    var c = cvData[cvTab];
    if (!c) {
        body.innerHTML = '<div class="cv-empty">// No output yet</div>';
        return;
    }
    if (cvTab === 'cpp') {
        body.innerHTML = '<div class="cv-code">' + hlCpp(c) + '</div>';
    } else if (cvTab === 'json') {
        body.innerHTML = '<div class="cv-code">' + hlJson(c) + '</div>';
    } else {
        body.innerHTML = '<div class="cv-code">' + c + '</div>';
    }
}

function showCV(tab) {
    cvTab = tab;
    ['cpp', 'json'].forEach(function(x) {
        var el = document.getElementById('cvt-' + x);
        if (el) { el.classList.toggle('active', x === tab); }
    });
    if (!cvOpen) {
        cvOpen = true;
        var cv = document.getElementById('cv');
        if (cv) { cv.classList.add('open'); }
        var btn = document.getElementById('btn-cv');
        if (btn) { btn.classList.add('active'); }
        var rh = document.getElementById('cv-resize');
        if (rh) { rh.classList.add('active'); }
    }
    renderCV();
}

/* ═══ CODE VIEWER DRAG-RESIZE ═══ */
var cvDragging = false;
var cvStartY = 0;
var cvStartH = 0;

if (typeof document !== 'undefined') {
    var cvResizeHandle = document.getElementById('cv-resize');
    if (cvResizeHandle) {
        cvResizeHandle.addEventListener('mousedown', function(e) {
            if (!cvOpen) { return; }
            e.preventDefault();
            cvDragging = true;
            cvStartY = e.clientY;
            var cv = document.getElementById('cv');
            cvStartH = cv ? cv.offsetHeight : 300;
            document.body.style.cursor = 'ns-resize';
            document.body.style.userSelect = 'none';
        });
    }
    document.addEventListener('mousemove', function(e) {
        if (!cvDragging) { return; }
        var delta = cvStartY - e.clientY;
        var newH = Math.max(80, Math.min(cvStartH + delta, window.innerHeight - 150));
        var cv = document.getElementById('cv');
        if (cv) {
            cv.style.height = newH + 'px';
        }
    });
    document.addEventListener('mouseup', function() {
        if (cvDragging) {
            cvDragging = false;
            document.body.style.cursor = '';
            document.body.style.userSelect = '';
        }
    });
}

function copyCV() {
    var c = cvData[cvTab];
    /* If cpp tab is empty, generate it now */
    if (cvTab === 'cpp' && !c) {
        if (!fsm.states.length) { toast('No states to copy', 'err'); return; }
        c = buildCpp();
        cvData.cpp = c;
        renderCV();
    }
    /* If json tab is empty, generate it now */
    if (cvTab === 'json' && !c) {
        c = buildJson();
        cvData.json = c;
        renderCV();
    }
    if (!c) { toast('Nothing to copy — run Generate C++ first', 'err'); return; }
    /* Strip HTML tags */
    var plain = c.replace(/<[^>]+>/g, '')
        .replace(/&lt;/g, '<')
        .replace(/&gt;/g, '>')
        .replace(/&amp;/g, '&')
        .replace(/&quot;/g, '"');
    navigator.clipboard.writeText(plain).then(function() {
        toast('Copied to clipboard ✓', 'ok');
    }).catch(function() {
        /* Fallback */
        var ta = document.createElement('textarea');
        ta.value = plain;
        ta.style.position = 'fixed';
        ta.style.opacity = '0';
        document.body.appendChild(ta);
        ta.select();
        document.execCommand('copy');
        document.body.removeChild(ta);
        toast('Copied to clipboard ✓', 'ok');
    });
}

function hlCpp(code) {
    return code.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
        .replace(/(\/\/[^\n]*)/g, '<span class="hl-cmt">$1</span>')
        .replace(/\b(class|struct|enum|public|private|protected|const|void|bool|int|uint8_t|return|if|else|switch|case|break|default|true|false|nullptr)\b/g, '<span class="hl-kw">$1</span>')
        .replace(/(#\w+)/g, '<span class="hl-pp">$1</span>')
        .replace(/\b([A-Z][A-Za-z0-9_]*)\b(?=\s*[:{(])/g, '<span class="hl-type">$1</span>');
}

function hlJson(c) {
    return c.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
        .replace(/"([^"]+)":/g, '"<span class="hl-kw">$1</span>":')
        .replace(/: "([^"]*)"/g, ': "<span class="hl-str">$1</span>"')
        .replace(/: (\d+\.?\d*)/g, ': <span class="hl-num">$1</span>')
        .replace(/: (true|false|null)/g, ': <span class="hl-type">$1</span>');
}

/* ═══ MODES ═══ */
function setMode(m) {
    mode = m;
    conFrom = null;
    ['sel', 'con', 'del'].forEach(function(x) {
        var el = document.getElementById('m' + x);
        if (el) { el.classList.toggle('active', x === m); }
    });
    var cw = document.getElementById('cw');
    if (cw) {
        cw.style.cursor = m === 'con' ? 'crosshair' : m === 'del' ? 'not-allowed' : 'default';
    }
}

/* ═══ MODALS ═══ */
var mcb = null;

function openModal(title, body, label, cb) {
    var mt = document.getElementById('mt');
    if (mt) { mt.textContent = title; }
    var mb = document.getElementById('mb');
    if (mb) { mb.innerHTML = body; }
    var okBtn = document.querySelector('.btn-ok');
    if (okBtn) { okBtn.textContent = label || 'Confirm'; }
    var mo = document.getElementById('mo');
    if (mo) { mo.classList.add('open'); }
    mcb = cb;
    setTimeout(function() {
        var f = document.querySelector('#mb input');
        if (f) { f.focus(); }
    }, 60);
}

function closeModal() {
    var mo = document.getElementById('mo');
    if (mo) { mo.classList.remove('open'); }
    mcb = null;
}

function modalOK() {
    if (mcb) { mcb(); }
}

if (typeof document !== 'undefined') {
    var moEl = document.getElementById('mo');
    if (moEl) {
        moEl.addEventListener('click', function(e) {
            if (e.target === moEl) { closeModal(); }
        });
    }
}

function openAddState() {
    var n = 'S' + (fsm.states.length + 1);
    openModal('Add State',
        '<div class="mf"><label>Name</label><input id="mn" value="' + n + '"></div>' +
        '<div class="mf"><label>Type</label><select id="mtp">' +
        '<option value="normal">Normal</option>' +
        '<option value="initial">Initial</option>' +
        '<option value="final">Final</option>' +
        '</select></div>',
        'Add State',
        function() {
            var nm = document.getElementById('mn').value.trim();
            var tp = document.getElementById('mtp').value;
            if (!nm) { return; }
            snap();
            var id = nid();
            var w = document.getElementById('cw').getBoundingClientRect();
            var cx = (w.width / 2 - panX) / zoom;
            var cy = (w.height / 2 - panY) / zoom;
            fsm.states.push({
                id: id, name: nm, type: tp,
                x: cx + (Math.random() - .5) * 160,
                y: cy + (Math.random() - .5) * 120,
                onEntry: '', onExit: '', entryGuard: '', exitGuard: '',
                internalEvents: [], comment: '', history: 'none'
            });
            if (tp === 'initial' && !fsm.initial) { fsm.initial = id; }
            renderAll();
            closeModal();
        }
    );
}

function openAddTransition() {
    if (fsm.states.length < 2) { toast('Need at least 2 states', 'err'); return; }
    var opts = fsm.states.map(function(s) {
        return '<option value="' + s.id + '">' + s.name + '</option>';
    }).join('');
    openModal('Add Transition',
        '<div class="mf"><label>From</label><select id="mfr">' + opts + '</select></div>' +
        '<div class="mf"><label>To</label><select id="mto">' + opts + '</select></div>' +
        '<div class="mf"><label>Event</label><input id="mev" placeholder="e.g. START"></div>' +
        '<div class="mf"><label>Guard (optional)</label><input id="mgd" placeholder="e.g. flag == true"></div>' +
        '<div class="mf"><label>Action (optional)</label><input id="mac" placeholder="e.g. onStart()"></div>',
        'Add',
        function() {
            snap();
            fsm.transitions.push({
                id: nid(),
                from: document.getElementById('mfr').value,
                to: document.getElementById('mto').value,
                event: document.getElementById('mev').value.trim(),
                guard: document.getElementById('mgd').value.trim(),
                action: document.getElementById('mac').value.trim()
            });
            renderAll();
            closeModal();
        }
    );
}

function openConnectModal(fid, tid) {
    var fn = '';
    var tn = '';
    fsm.states.forEach(function(s) {
        if (s.id === fid) { fn = s.name; }
        if (s.id === tid) { tn = s.name; }
    });
    openModal('Connect: ' + fn + ' → ' + tn,
        '<div class="mf"><label>Event</label><input id="mev" placeholder="e.g. TRIGGER"></div>' +
        '<div class="mf"><label>Guard (optional)</label><input id="mgd" placeholder="e.g. x > 0"></div>' +
        '<div class="mf"><label>Action (optional)</label><input id="mac" placeholder="e.g. doAction()"></div>',
        'Connect',
        function() {
            snap();
            fsm.transitions.push({
                id: nid(),
                from: fid, to: tid,
                event: document.getElementById('mev').value.trim(),
                guard: document.getElementById('mgd').value.trim(),
                action: document.getElementById('mac').value.trim()
            });
            renderAll();
            closeModal();
        }
    );
}

/* ═══ PROPERTIES ═══ */

/** Currently selected inline editor state ID */
var inlineEditId = null;

/**
 * Show state properties in the right panel when a state is selected.
 * @param {string} stateId - The ID of the selected state.
 */
function showStateProperties(stateId) {
    var state = fsm.states.find(function(s) { return s.id === stateId; });
    if (!state) { return; }
    inlineEditId = stateId;
    switchTab('states');
    renderSL();
}

function switchTab(t) {
    ['fsm', 'states', 'trans'].forEach(function(x) {
        var tab = document.getElementById('tab-' + x);
        var content = document.getElementById('tc-' + x);
        if (tab) { tab.classList.toggle('active', x === t); }
        if (content) { content.style.display = x === t ? 'block' : 'none'; }
    });
    if (t === 'states') { renderSL(); }
    if (t === 'trans') { renderTL(); }
}

function refreshProps() {
    var sel = document.getElementById('fsm-init');
    if (sel) {
        sel.innerHTML = '<option value="">— none —</option>' +
            fsm.states.map(function(s) {
                return '<option value="' + s.id + '"' +
                    (fsm.initial === s.id ? ' selected' : '') + '>' +
                    s.name + '</option>';
            }).join('');
    }
    var stS = document.getElementById('st-s');
    if (stS) { stS.textContent = fsm.states.length; }
    var stT = document.getElementById('st-t');
    if (stT) { stT.textContent = fsm.transitions.length; }
    var stF = document.getElementById('st-f');
    if (stF) { stF.textContent = fsm.states.filter(function(s) { return s.type === 'final'; }).length; }
    var stE = document.getElementById('st-e');
    if (stE) {
        var evSet = {};
        fsm.transitions.forEach(function(t) {
            if (t.event) { evSet[t.event] = true; }
        });
        stE.textContent = Object.keys(evSet).length;
    }
    renderSL();
    renderTL();
}

function renderSL() {
    var ul = document.getElementById('sl-ui');
    if (!ul) { return; }
    if (!fsm.states.length) {
        ul.innerHTML = '<div style="color:var(--text3);font-size:12px;padding:4px 0;">No states yet.</div>';
        return;
    }
    ul.innerHTML = '';

    /* Render compact state list */
    fsm.states.forEach(function(s) {
        var d = document.createElement('div');
        d.className = 'sc' + (s.id === inlineEditId ? ' sel' : '');
        d.innerHTML =
            '<div class="sc-name">' + s.name + '</div>' +
            '<div class="sc-type">' + s.type + '</div>' +
            '<button class="sc-del" title="Delete">✕</button>';
        d.onclick = function() {
            selId = s.id;
            inlineEditId = s.id;
            renderAll();
            renderSL();
        };
        d.querySelector('.sc-del').onclick = function(e) {
            e.stopPropagation();
            snap();
            fsm.states = fsm.states.filter(function(x) { return x.id !== s.id; });
            fsm.transitions = fsm.transitions.filter(function(t) {
                return t.from !== s.id && t.to !== s.id;
            });
            if (inlineEditId === s.id) { inlineEditId = null; }
            renderAll();
        };
        ul.appendChild(d);
    });

    /* If a state is selected, render inline editor below the list */
    if (inlineEditId) {
        var state = fsm.states.find(function(s) { return s.id === inlineEditId; });
        if (state) {
            renderInlineStateEditor(ul, state);
        }
    }
}

/**
 * Render the inline state property editor below the state list.
 * Shows: General info, Entry actions, Exit actions, Internal events, Connected Events.
 * @param {HTMLElement} container - Parent element to append to.
 * @param {object} state - The state object.
 */
function renderInlineStateEditor(container, state) {
    var editor = document.createElement('div');
    editor.className = 'ise';

    /* ── Header ── */
    var hdr = document.createElement('div');
    hdr.className = 'ise-hdr';
    hdr.innerHTML = '<div class="ise-title">' + state.name + '</div>' +
        '<button class="ise-edit" onclick="openSE(\'' + state.id + '\')" title="Open full editor">⚙</button>' +
        '<button class="ise-close" title="Close">✕</button>';
    hdr.querySelector('.ise-close').onclick = function() {
        inlineEditId = null;
        renderSL();
    };
    editor.appendChild(hdr);

    /* ── General Section ── */
    var gen = createEditorSection('General', true);
    gen.body.innerHTML =
        '<div class="ise-row">' +
            '<div class="ise-field">' +
                '<label class="ise-lbl">Name</label>' +
                '<input class="ise-inp" id="ise-name" value="' + esc(state.name) + '">' +
            '</div>' +
            '<div class="ise-field">' +
                '<label class="ise-lbl">Type</label>' +
                '<select class="ise-inp" id="ise-type">' +
                    '<option value="normal"' + (state.type === 'normal' ? ' selected' : '') + '>Normal</option>' +
                    '<option value="initial"' + (state.type === 'initial' ? ' selected' : '') + '>Initial</option>' +
                    '<option value="final"' + (state.type === 'final' ? ' selected' : '') + '>Final</option>' +
                '</select>' +
            '</div>' +
        '</div>' +
        '<div class="ise-row">' +
            '<div class="ise-field">' +
                '<label class="ise-lbl">History</label>' +
                '<select class="ise-inp" id="ise-history">' +
                    '<option value="none"' + (state.history === 'none' ? ' selected' : '') + '>None</option>' +
                    '<option value="shallow"' + (state.history === 'shallow' ? ' selected' : '') + '>Shallow</option>' +
                    '<option value="deep"' + (state.history === 'deep' ? ' selected' : '') + '>Deep</option>' +
                '</select>' +
            '</div>' +
        '</div>' +
        '<div class="ise-field">' +
            '<label class="ise-lbl">Comment</label>' +
            '<input class="ise-inp" id="ise-comment" value="' + esc(state.comment || '') + '" placeholder="Optional note...">' +
        '</div>';
    bindISEInput(gen.body, state);
    editor.appendChild(gen.el);

    /* ── On Entry Section ── */
    var entry = createEditorSection('On Entry', !!(state.onEntry || state.entryGuard));
    var entryHtml =
        '<div class="ise-field">' +
            '<label class="ise-lbl">Entry Actions</label>' +
            '<textarea class="ise-ta" id="ise-onentry" rows="3" placeholder="e.g. startTimer();\nsetLed(ON);">' + esc(state.onEntry || '') + '</textarea>' +
        '</div>' +
        '<div class="ise-field">' +
            '<label class="ise-lbl">Entry Guard</label>' +
            '<input class="ise-inp" id="ise-entry-guard" value="' + esc(state.entryGuard || '') + '" placeholder="Condition for entry...">' +
        '</div>';
    entry.body.innerHTML = entryHtml;
    bindISEInput(entry.body, state);
    editor.appendChild(entry.el);

    /* ── On Exit Section ── */
    var exit = createEditorSection('On Exit', !!(state.onExit || state.exitGuard));
    var exitHtml =
        '<div class="ise-field">' +
            '<label class="ise-lbl">Exit Actions</label>' +
            '<textarea class="ise-ta" id="ise-onexit" rows="3" placeholder="e.g. stopTimer();\ncleanup();">' + esc(state.onExit || '') + '</textarea>' +
        '</div>' +
        '<div class="ise-field">' +
            '<label class="ise-lbl">Exit Guard</label>' +
            '<input class="ise-inp" id="ise-exit-guard" value="' + esc(state.exitGuard || '') + '" placeholder="Condition for exit...">' +
        '</div>';
    exit.body.innerHTML = exitHtml;
    bindISEInput(exit.body, state);
    editor.appendChild(exit.el);

    /* ── Internal Events Section ── */
    var intev = createEditorSection('Internal Events (' + (state.internalEvents || []).length + ')',
        (state.internalEvents || []).length > 0);
    renderISEInternalEvents(intev.body, state);
    editor.appendChild(intev.el);

    /* ── Connected Events (transitions) Section ── */
    var connTrans = fsm.transitions.filter(function(t) {
        return t.from === state.id || t.to === state.id;
    });
    var connev = createEditorSection('Connected Events (' + connTrans.length + ')', connTrans.length > 0);
    renderISEConnectedEvents(connev.body, state, connTrans);
    editor.appendChild(connev.el);

    container.appendChild(editor);
}

/**
 * Create a collapsible section for the inline state editor.
 * @param {string} title - Section title.
 * @param {boolean} open - Whether to start open.
 * @returns {{ el: HTMLElement, body: HTMLElement }}
 */
function createEditorSection(title, open) {
    var section = document.createElement('div');
    section.className = 'ise-section';

    var header = document.createElement('div');
    header.className = 'ise-sec-hd';
    header.innerHTML = '<span class="ise-sec-arrow">' + (open ? '▴' : '▸') + '</span>' +
        '<span class="ise-sec-title">' + title + '</span>';

    var body = document.createElement('div');
    body.className = 'ise-sec-body';
    body.style.display = open ? 'block' : 'none';

    header.onclick = function() {
        var isOpen = body.style.display !== 'none';
        body.style.display = isOpen ? 'none' : 'block';
        header.querySelector('.ise-sec-arrow').textContent = isOpen ? '▸' : '▴';
    };

    section.appendChild(header);
    section.appendChild(body);
    return { el: section, body: body };
}

/**
 * Bind live-edit events from inline editor inputs back to the state model.
 * @param {HTMLElement} container - Container with inputs.
 * @param {object} state - The state object.
 */
function bindISEInput(container, state) {
    setTimeout(function() {
        var nameEl = container.querySelector('#ise-name');
        if (nameEl) {
            nameEl.addEventListener('change', function() {
                var v = this.value.trim();
                if (v) { state.name = v; renderAll(); }
            });
        }
        var typeEl = container.querySelector('#ise-type');
        if (typeEl) {
            typeEl.addEventListener('change', function() {
                state.type = this.value;
                if (this.value === 'initial') { fsm.initial = state.id; }
                renderAll();
            });
        }
        var histEl = container.querySelector('#ise-history');
        if (histEl) {
            histEl.addEventListener('change', function() {
                state.history = this.value;
            });
        }
        var commentEl = container.querySelector('#ise-comment');
        if (commentEl) {
            commentEl.addEventListener('change', function() {
                state.comment = this.value.trim();
            });
        }
        var entryEl = container.querySelector('#ise-onentry');
        if (entryEl) {
            entryEl.addEventListener('input', function() {
                state.onEntry = this.value.trim();
            });
        }
        var entryGuardEl = container.querySelector('#ise-entry-guard');
        if (entryGuardEl) {
            entryGuardEl.addEventListener('change', function() {
                state.entryGuard = this.value.trim();
            });
        }
        var exitEl = container.querySelector('#ise-onexit');
        if (exitEl) {
            exitEl.addEventListener('input', function() {
                state.onExit = this.value.trim();
            });
        }
        var exitGuardEl = container.querySelector('#ise-exit-guard');
        if (exitGuardEl) {
            exitGuardEl.addEventListener('change', function() {
                state.exitGuard = this.value.trim();
            });
        }
    }, 0);
}

/**
 * Render the internal events table inside the inline editor.
 * @param {HTMLElement} body - Section body element.
 * @param {object} state - The state object.
 */
function renderISEInternalEvents(body, state) {
    var events = state.internalEvents || [];
    if (events.length === 0) {
        body.innerHTML = '<div class="ise-empty">No internal events</div>';
    } else {
        var table = '<table class="ise-ie-table">' +
            '<thead><tr><th>Event</th><th>Guard</th><th>Action</th><th></th></tr></thead><tbody>';
        events.forEach(function(ev, i) {
            table += '<tr>' +
                '<td><input class="ise-ie-inp" value="' + esc(ev.event || '') + '" data-idx="' + i + '" data-field="event"></td>' +
                '<td><input class="ise-ie-inp" value="' + esc(ev.guard || '') + '" data-idx="' + i + '" data-field="guard"></td>' +
                '<td><input class="ise-ie-inp" value="' + esc(ev.action || '') + '" data-idx="' + i + '" data-field="action"></td>' +
                '<td><button class="ise-ie-del" data-idx="' + i + '" title="Remove">✕</button></td>' +
                '</tr>';
        });
        table += '</tbody></table>';
        body.innerHTML = table;
    }

    var addBtn = document.createElement('button');
    addBtn.className = 'ise-ie-add';
    addBtn.textContent = '＋ Add internal event';
    addBtn.onclick = function() {
        state.internalEvents = state.internalEvents || [];
        state.internalEvents.push({ event: '', guard: '', action: '' });
        renderSL();
    };
    body.appendChild(addBtn);

    /* Bind events */
    setTimeout(function() {
        var inputs = body.querySelectorAll('.ise-ie-inp');
        inputs.forEach(function(inp) {
            inp.addEventListener('change', function() {
                var idx = parseInt(this.getAttribute('data-idx'), 10);
                var field = this.getAttribute('data-field');
                if (state.internalEvents[idx]) {
                    state.internalEvents[idx][field] = this.value.trim();
                }
            });
        });
        var delBtns = body.querySelectorAll('.ise-ie-del');
        delBtns.forEach(function(btn) {
            btn.addEventListener('click', function() {
                var idx = parseInt(this.getAttribute('data-idx'), 10);
                state.internalEvents.splice(idx, 1);
                renderSL();
            });
        });
    }, 0);
}

/**
 * Render connected events (transitions to/from this state) in the inline editor.
 * @param {HTMLElement} body - Section body element.
 * @param {object} state - The state object.
 * @param {Array} trans - Filtered transitions for this state.
 */
function renderISEConnectedEvents(body, state, trans) {
    if (trans.length === 0) {
        body.innerHTML = '<div class="ise-empty">No connected transitions</div>';
        return;
    }
    var html = '<div class="ise-conn-list">';
    trans.forEach(function(t) {
        var fromName = '';
        var toName = '';
        fsm.states.forEach(function(s) {
            if (s.id === t.from) { fromName = s.name; }
            if (s.id === t.to) { toName = s.name; }
        });
        var direction = (t.from === state.id) ? 'outgoing' : 'incoming';
        var dirIcon = (t.from === state.id) ? '→' : '←';
        var dirLabel = (t.from === state.id) ? toName : fromName;
        html += '<div class="ise-conn-item ' + direction + '">' +
            '<div class="ise-conn-dir">' +
                '<span class="ise-conn-icon">' + dirIcon + '</span>' +
                '<span class="ise-conn-target">' + dirLabel + '</span>' +
            '</div>' +
            '<div class="ise-conn-details">' +
                '<span class="ise-conn-ev">⚡ ' + (t.event || '—') + '</span>' +
                (t.guard ? '<span class="ise-conn-gd">🛡 ' + t.guard + '</span>' : '') +
                (t.action ? '<span class="ise-conn-ac">▶ ' + t.action + '</span>' : '') +
            '</div>' +
        '</div>';
    });
    html += '</div>';
    body.innerHTML = html;
}

/**
 * Escape HTML special characters.
 * @param {string} s - Input string.
 * @returns {string} Escaped string.
 */
function esc(s) {
    if (!s) { return ''; }
    return s.replace(/&/g, '&amp;').replace(/</g, '&lt;')
        .replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

function renderTL() {
    var ul = document.getElementById('tl-ui');
    if (!ul) { return; }
    if (!fsm.transitions.length) {
        ul.innerHTML = '<div style="color:var(--text3);font-size:12px;padding:4px 0;">No transitions yet.</div>';
        return;
    }
    ul.innerHTML = '';
    fsm.transitions.forEach(function(t) {
        var fn = '';
        var tn = '';
        fsm.states.forEach(function(s) {
            if (s.id === t.from) { fn = s.name; }
            if (s.id === t.to) { tn = s.name; }
        });
        var d = document.createElement('div');
        d.className = 'ti';
        d.innerHTML =
            '<div class="ti-ft">' + fn + ' <span class="ti-arr">→</span> ' + tn + '</div>' +
            '<div class="ti-ev">' + (t.event || '(no event)') +
            (t.guard ? ' [' + t.guard + ']' : '') +
            (t.action ? ' / ' + t.action : '') + '</div>';
        d.onclick = function() {
            selId = t.id;
            renderAll();
        };
        ul.appendChild(d);
    });
}

function updSB() {
    var sc1 = document.getElementById('sc1');
    var sc2 = document.getElementById('sc2');
    if (sc1) { sc1.textContent = fsm.states.length + ' state' + (fsm.states.length !== 1 ? 's' : ''); }
    if (sc2) { sc2.textContent = fsm.transitions.length + ' transition' + (fsm.transitions.length !== 1 ? 's' : ''); }
}

/* ═══ IMPORT MODAL ═══ */

function openImportMenu() {
    var overlay = document.getElementById('imp-overlay');
    if (overlay) { overlay.classList.add('open'); }
    var status = document.getElementById('imp-status');
    if (status) { status.textContent = ''; }
    var fi = document.getElementById('imp-files');
    if (fi) { fi.value = ''; }
}

function closeImportMenu() {
    var overlay = document.getElementById('imp-overlay');
    if (overlay) { overlay.classList.remove('open'); }
}

function processImport() {
    var fi = document.getElementById('imp-files');
    var status = document.getElementById('imp-status');
    if (!fi || !fi.files || fi.files.length === 0) {
        if (status) { status.textContent = 'Please select at least one file to import.'; }
        return;
    }
    
    var files = Array.from(fi.files);
    var contents = [];
    var loadedCount = 0;
    
    files.forEach(function(f, i) {
        var reader = new FileReader();
        reader.onload = function(e) {
            contents[i] = e.target.result;
            loadedCount++;
            if (loadedCount === files.length) {
                // Combine contents
                var combined = contents.join('\n\n/* --- FILE BOUNDARY --- */\n\n');
                finalizeImport(combined);
            }
        };
        reader.readAsText(f);
    });
}

function finalizeImport(combinedText) {
    if (typeof parser === 'undefined' || !parser.parseInput) {
        var status = document.getElementById('imp-status');
        if (status) { status.textContent = 'Parser module not loaded.'; }
        return;
    }
    var res = parser.parseInput(combinedText);
    if (res.errors && res.errors.length > 0) {
        var status = document.getElementById('imp-status');
        if (status) { status.textContent = 'Parse error: ' + res.errors[0]; }
        return;
    }
    
    if (res.states && res.states.length > 0) {
        resetFSM();
        fsm.name = res.name || 'ImportedFSM';
        fsm.initial = res.initial || '';
        fsm.states = res.states;
        fsm.transitions = res.transitions || [];
        
        // Auto-layout if missing coordinates
        fsm.states.forEach(function(s, i) {
            if (s.x === undefined) { s.x = 100 + (i % 4) * 160; }
            if (s.y === undefined) { s.y = 100 + Math.floor(i / 4) * 120; }
        });
        
        snap();
        closeImportMenu();
        renderAll();
        refreshProps();
        if (typeof toast === 'function') { toast('Imported ' + fsm.states.length + ' states ✓', 'ok'); }
    } else {
        var status = document.getElementById('imp-status');
        if (status) { status.textContent = 'No FSM states could be found in the files.'; }
    }
}
