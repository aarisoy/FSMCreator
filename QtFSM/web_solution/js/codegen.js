// codegen.js — C++ and JSON code generation with section markers
//
// Reads the `fsm` model and produces output code with
// FSM_HEADER_BEGIN/END and FSM_SOURCE_BEGIN/END markers.
//
// Key functions:
//   buildCpp()          — generate C++ with section markers
//   buildJson()         — serialize fsm to JSON string
//   generateCppAndShow()— build C++ and show in code viewer
//   exportCpp/Json/Dsl()— download as file
//   copyCpp/Json()      — copy to clipboard

/* ═══ CONSTANTS ═══ */
var MARKER_HEADER_BEGIN = '// === FSM_HEADER_BEGIN ===';
var MARKER_HEADER_END = '// === FSM_HEADER_END ===';
var MARKER_SOURCE_BEGIN = '// === FSM_SOURCE_BEGIN ===';
var MARKER_SOURCE_END = '// === FSM_SOURCE_END ===';

/* ═══ BUILD C++ HEADER ═══ */

/**
 * Build the C++ header section content.
 * @param {object} fsmData - The FSM data object.
 * @param {string} ns - Optional namespace.
 * @returns {string} Header section content.
 */
function buildHeaderSection(fsmData, ns) {
    if (!fsmData || !fsmData.states) {
        return '';
    }

    var name = fsmData.name || 'MyFSM';
    var states = fsmData.states;
    var transitions = fsmData.transitions || [];

    var initS = null;
    for (var i = 0; i < states.length; i++) {
        if (states[i].id === fsmData.initial) {
            initS = states[i];
            break;
        }
    }
    if (!initS && states.length > 0) {
        initS = states[0];
    }

    var evs = [];
    var evSet = {};
    transitions.forEach(function(t) {
        if (t.event && !evSet[t.event]) {
            evSet[t.event] = true;
            evs.push(t.event);
        }
    });

    var stateEnumEntries = states.map(function(s) {
        return '        ' + s.name.toUpperCase();
    }).join(',\n');

    var eventEnumEntries = evs.map(function(e) {
        return '        ' + e.toUpperCase();
    }).join(',\n');

    var fc = states
        .filter(function(s) { return s.type === 'final'; })
        .map(function(s) { return 'state_ == State::' + s.name.toUpperCase(); })
        .join(' || ') || 'false';

    var stateNameCases = states.map(function(s) {
        return '        case State::' + s.name.toUpperCase() +
            ': return "' + s.name + '";';
    }).join('\n');

    var nsOpen = ns ? ('namespace ' + ns + ' {\n\n') : '';
    var nsClose = ns ? ('\n} // namespace ' + ns + '\n') : '';

    var lines = [];
    lines.push('#pragma once');
    lines.push('#include <cstdint>');
    lines.push('');
    lines.push(nsOpen + 'class ' + name + ' {');
    lines.push('public:');
    lines.push('    enum class State : uint8_t {');
    lines.push(stateEnumEntries);
    lines.push('    };');
    lines.push('');
    lines.push('    enum class Event : uint8_t {');
    lines.push(eventEnumEntries || '        // No events');
    lines.push('    };');
    lines.push('');
    lines.push('    ' + name + '() : state_(State::' +
        (initS ? initS.name.toUpperCase() : 'UNKNOWN') + ') {}');
    lines.push('');
    lines.push('    bool process(Event event);');
    lines.push('    State getState() const { return state_; }');
    lines.push('    bool isFinal() const { return (' + fc + '); }');
    lines.push('    void reset() { state_ = State::' +
        (initS ? initS.name.toUpperCase() : 'UNKNOWN') + '; }');
    lines.push('    const char* stateName() const;');
    lines.push('    void onEntry();');
    lines.push('    void onExit();');
    lines.push('');
    lines.push('private:');
    lines.push('    State state_;');
    lines.push('};' + nsClose);

    return lines.join('\n');
}

/* ═══ BUILD C++ SOURCE ═══ */

/**
 * Build the C++ source section content.
 * @param {object} fsmData - The FSM data object.
 * @param {string} ns - Optional namespace.
 * @returns {string} Source section content.
 */
function buildSourceSection(fsmData, ns) {
    if (!fsmData || !fsmData.states) {
        return '';
    }

    var name = fsmData.name || 'MyFSM';
    var states = fsmData.states;
    var transitions = fsmData.transitions || [];

    var initS = null;
    for (var i = 0; i < states.length; i++) {
        if (states[i].id === fsmData.initial) {
            initS = states[i];
            break;
        }
    }
    if (!initS && states.length > 0) {
        initS = states[0];
    }

    var nsOpen = ns ? ('namespace ' + ns + ' {\n\n') : '';
    var nsClose = ns ? ('\n} // namespace ' + ns + '\n') : '';

    /* Build process() switch cases */
    var transitionCases = '';
    transitions.forEach(function(t) {
        var fromName = '';
        var toName = '';
        for (var j = 0; j < states.length; j++) {
            if (states[j].id === t.from) { fromName = states[j].name.toUpperCase(); }
            if (states[j].id === t.to) { toName = states[j].name.toUpperCase(); }
        }
        if (!fromName || !toName) { return; }

        var ev = (t.event || '').toUpperCase();
        var gd = t.guard ? ('\n            // guard: ' + t.guard) : '';
        var ac = t.action ? ('\n            ' + t.action + ';') : '';

        transitionCases +=
            '        case State::' + fromName + ':\n' +
            '            if (event == Event::' + ev + ') {' + gd + ac + '\n' +
            '                onExit(); state_ = State::' + toName + '; onEntry();\n' +
            '                return true;\n' +
            '            } break;\n';
    });

    /* Build onEntry switch cases */
    var entryHandlers = '';
    states.forEach(function(s) {
        var sn = s.name.toUpperCase();
        var entry = (s.onEntry || '').trim();
        if (entry) {
            entryHandlers +=
                '        case State::' + sn + ':\n' +
                entry.split('\n').map(function(l) {
                    return '            ' + l.trim();
                }).join('\n') + '\n            break;\n';
        }
    });

    /* Build onExit switch cases */
    var exitHandlers = '';
    states.forEach(function(s) {
        var sn = s.name.toUpperCase();
        var exit = (s.onExit || '').trim();
        if (exit) {
            exitHandlers +=
                '        case State::' + sn + ':\n' +
                exit.split('\n').map(function(l) {
                    return '            ' + l.trim();
                }).join('\n') + '\n            break;\n';
        }
    });

    /* Build stateName switch cases */
    var stateNameCases = states.map(function(s) {
        return '        case State::' + s.name.toUpperCase() +
            ': return "' + s.name + '";';
    }).join('\n');

    var lines = [];
    lines.push('#include "' + name + '.h"');
    lines.push('');
    lines.push(nsOpen + 'bool ' + name + '::process(Event event) {');
    lines.push('    switch (state_) {');
    lines.push(transitionCases || '    // No transitions');
    lines.push('    default: break;');
    lines.push('    }');
    lines.push('    return false;');
    lines.push('}');
    lines.push('');
    lines.push('const char* ' + name + '::stateName() const {');
    lines.push('    switch (state_) {');
    lines.push(stateNameCases);
    lines.push('    default: return "Unknown";');
    lines.push('    }');
    lines.push('}');
    lines.push('');
    lines.push('void ' + name + '::onEntry() {');
    lines.push('    switch (state_) {');
    lines.push(entryHandlers || '    // No entry actions defined');
    lines.push('    default: break;');
    lines.push('    }');
    lines.push('}');
    lines.push('');
    lines.push('void ' + name + '::onExit() {');
    lines.push('    switch (state_) {');
    lines.push(exitHandlers || '    // No exit actions defined');
    lines.push('    default: break;');
    lines.push('    }');
    lines.push('}' + nsClose);

    return lines.join('\n');
}

/* ═══ BUILD COMPLETE C++ ═══ */

/**
 * Build full C++ output with section markers.
 * @param {object} fsmData - The FSM data object (defaults to global fsm).
 * @param {string} ns - Optional namespace.
 * @returns {string} Complete C++ with markers.
 */
function buildCpp(fsmData, ns) {
    /* In browser context, fall back to globals */
    var data = fsmData || (typeof fsm !== 'undefined' ? fsm : null);
    if (!data) { return ''; }

    var namespace = ns;
    if (typeof namespace === 'undefined' && typeof document !== 'undefined') {
        var nsEl = document.getElementById('fsm-ns');
        namespace = nsEl ? nsEl.value.trim() : '';
    }
    namespace = namespace || '';

    var name = data.name || 'MyFSM';
    var desc = '';
    if (typeof document !== 'undefined') {
        var descEl = document.getElementById('fsm-desc');
        desc = descEl ? descEl.value.trim() : '';
    }

    var header = buildHeaderSection(data, namespace);
    var source = buildSourceSection(data, namespace);

    var lines = [];
    lines.push('// Generated by FSM Creator');
    lines.push('// FSM:    ' + name);
    lines.push('// States: ' + data.states.length +
        ' | Transitions: ' + (data.transitions || []).length);
    if (desc) {
        lines.push('// Desc:   ' + desc);
    }
    lines.push('');
    lines.push(MARKER_HEADER_BEGIN);
    lines.push(header);
    lines.push(MARKER_HEADER_END);
    lines.push('');
    lines.push(MARKER_SOURCE_BEGIN);
    lines.push(source);
    lines.push(MARKER_SOURCE_END);

    return lines.join('\n');
}

/* ═══ BUILD JSON ═══ */

/**
 * Build JSON representation of the FSM.
 * @param {object} fsmData - The FSM data object (defaults to global fsm).
 * @returns {string} JSON string.
 */
function buildJson(fsmData) {
    var data = fsmData || (typeof fsm !== 'undefined' ? fsm : null);
    if (!data) { return '{}'; }

    var ns = '';
    var desc = '';
    if (typeof document !== 'undefined') {
        var nsEl = document.getElementById('fsm-ns');
        ns = nsEl ? nsEl.value.trim() : '';
        var descEl = document.getElementById('fsm-desc');
        desc = descEl ? descEl.value.trim() : '';
    }

    var initialName = null;
    var states = data.states || [];
    for (var i = 0; i < states.length; i++) {
        if (states[i].id === data.initial) {
            initialName = states[i].name;
            break;
        }
    }

    var jsonObj = {
        fsm: {
            name: data.name,
            namespace: ns,
            description: desc,
            initial: initialName,
            states: states.map(function(s) {
                return {
                    name: s.name,
                    type: s.type,
                    history: s.history || 'none',
                    comment: s.comment || '',
                    position: { x: Math.round(s.x || 0), y: Math.round(s.y || 0) },
                    onEntry: s.onEntry || '',
                    onExit: s.onExit || '',
                    entryGuard: s.entryGuard || '',
                    exitGuard: s.exitGuard || '',
                    internalEvents: s.internalEvents || []
                };
            }),
            transitions: (data.transitions || []).map(function(t) {
                var fromName = '';
                var toName = '';
                for (var j = 0; j < states.length; j++) {
                    if (states[j].id === t.from) { fromName = states[j].name; }
                    if (states[j].id === t.to) { toName = states[j].name; }
                }
                return {
                    from: fromName,
                    to: toName,
                    event: t.event,
                    guard: t.guard,
                    action: t.action
                };
            })
        },
        meta: {
            generator: 'FSM Creator',
            version: '2.0'
        }
    };

    return JSON.stringify(jsonObj, null, 2);
}

/* ═══ BROWSER-ONLY UI FUNCTIONS ═══ */

function generateCppAndShow() {
    if (typeof fsm === 'undefined' || !fsm.states.length) {
        if (typeof toast === 'function') { toast('No states yet', 'err'); }
        return;
    }
    cvData.cpp = buildCpp();
    showCV('cpp');
    if (typeof toast === 'function') { toast('C++ generated ✓', 'ok'); }
}

/* ═══ EXPORT ═══ */
function dl(name, content, mime) {
    var b = new Blob([content], { type: mime });
    var a = document.createElement('a');
    a.href = URL.createObjectURL(b);
    a.download = name;
    a.click();
    URL.revokeObjectURL(a.href);
}

function toggleExpMenu(e) {
    e.stopPropagation();
    document.getElementById('exp-menu').classList.toggle('open');
}
if (typeof document !== 'undefined') {
    document.addEventListener('click', function() {
        var menu = document.getElementById('exp-menu');
        if (menu) { menu.classList.remove('open'); }
    });
}

function exportCpp() {
    if (typeof fsm === 'undefined' || !fsm.states.length) {
        if (typeof toast === 'function') { toast('No states', 'err'); }
        return;
    }
    var c = buildCpp();
    cvData.cpp = c;
    dl(fsm.name + '.h', c, 'text/plain');
    if (typeof toast === 'function') { toast('Exported ' + fsm.name + '.h ✓', 'ok'); }
    document.getElementById('exp-menu').classList.remove('open');
}

function exportJson() {
    var j = buildJson();
    cvData.json = j;
    dl(fsm.name + '.json', j, 'application/json');
    if (typeof toast === 'function') { toast('Exported ' + fsm.name + '.json ✓', 'ok'); }
    document.getElementById('exp-menu').classList.remove('open');
}

function exportDsl() {
    var d = '// FSM: ' + fsm.name + '\nFSM: ' + fsm.name + '\n';
    fsm.states.forEach(function(s) {
        d += 'STATE: ' + s.name + '\n';
    });
    var ini = fsm.states.find(function(s) { return s.id === fsm.initial; });
    if (ini) { d += 'INIT: ' + ini.name + '\n'; }
    fsm.states.filter(function(s) { return s.type === 'final'; })
        .forEach(function(s) { d += 'FINAL: ' + s.name + '\n'; });
    fsm.transitions.forEach(function(t) {
        var fn = '';
        var tn = '';
        fsm.states.forEach(function(s) {
            if (s.id === t.from) { fn = s.name; }
            if (s.id === t.to) { tn = s.name; }
        });
        d += 'TRANS: ' + fn + ' -> ' + tn + ' : ' + t.event +
            (t.guard ? ' [' + t.guard + ']' : '') +
            (t.action ? ' / ' + t.action : '') + '\n';
    });
    dl(fsm.name + '.txt', d, 'text/plain');
    if (typeof toast === 'function') { toast('Exported ' + fsm.name + '.txt ✓', 'ok'); }
    document.getElementById('exp-menu').classList.remove('open');
}

function copyCpp() {
    if (typeof fsm === 'undefined' || !fsm.states.length) {
        if (typeof toast === 'function') { toast('No states', 'err'); }
        return;
    }
    navigator.clipboard.writeText(buildCpp()).then(function() {
        if (typeof toast === 'function') { toast('C++ copied ✓', 'ok'); }
    });
    document.getElementById('exp-menu').classList.remove('open');
}

function copyJson() {
    navigator.clipboard.writeText(buildJson()).then(function() {
        if (typeof toast === 'function') { toast('JSON copied ✓', 'ok'); }
    });
    document.getElementById('exp-menu').classList.remove('open');
}

/* ═══ NODE.JS EXPORTS (for Jest testing) ═══ */
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        MARKER_HEADER_BEGIN: MARKER_HEADER_BEGIN,
        MARKER_HEADER_END: MARKER_HEADER_END,
        MARKER_SOURCE_BEGIN: MARKER_SOURCE_BEGIN,
        MARKER_SOURCE_END: MARKER_SOURCE_END,
        buildHeaderSection: buildHeaderSection,
        buildSourceSection: buildSourceSection,
        buildCpp: buildCpp,
        buildJson: buildJson
    };
}
