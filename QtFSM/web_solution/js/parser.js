// parser.js — FSM parser with comment-section markers
//
// Parses C++ code that uses section markers to separate
// header (.h) and source (.cpp) content:
//
//   // === FSM_HEADER_BEGIN ===
//   ... header content ...
//   // === FSM_HEADER_END ===
//   // === FSM_SOURCE_BEGIN ===
//   ... source content ...
//   // === FSM_SOURCE_END ===
//
// Also supports a simple DSL format as fallback.
//
// Entry point: parseInput(src)
// Then call:   applyParseResult(result)

/* ═══ CONSTANTS ═══ */
var MARKER_HEADER_BEGIN = '// === FSM_HEADER_BEGIN ===';
var MARKER_HEADER_END = '// === FSM_HEADER_END ===';
var MARKER_SOURCE_BEGIN = '// === FSM_SOURCE_BEGIN ===';
var MARKER_SOURCE_END = '// === FSM_SOURCE_END ===';

/* ═══ SECTION EXTRACTION ═══ */

/**
 * Extract content between two markers from source text.
 * @param {string} src - Full source text.
 * @param {string} beginMarker - Opening marker string.
 * @param {string} endMarker - Closing marker string.
 * @returns {string|null} Extracted content, or null if markers not found.
 */
function extractSection(src, beginMarker, endMarker) {
    if (typeof src !== 'string') {
        return null;
    }
    var beginIdx = src.indexOf(beginMarker);
    if (beginIdx === -1) {
        return null;
    }
    var contentStart = beginIdx + beginMarker.length;
    var endIdx = src.indexOf(endMarker, contentStart);
    if (endIdx === -1) {
        return null;
    }
    return src.substring(contentStart, endIdx).trim();
}

/**
 * Detect which format the input uses.
 * @param {string} src - Source text.
 * @returns {string} 'marked' | 'dsl' | 'unknown'
 */
function detectFormat(src) {
    if (typeof src !== 'string') {
        return 'unknown';
    }
    if (src.indexOf(MARKER_HEADER_BEGIN) !== -1 ||
        src.indexOf(MARKER_SOURCE_BEGIN) !== -1) {
        return 'marked';
    }
    if (/^\s*(FSM|STATE|TRANS|INIT|FINAL)\s*:/m.test(src)) {
        return 'dsl';
    }
    return 'unknown';
}

/* ═══ BLANK RESULT ═══ */

/**
 * Create a blank parse result.
 * @param {string} name - FSM name.
 * @returns {object} Blank result object.
 */
function blankResult(name) {
    return {
        name: name || 'MyFSM',
        initial: '',
        states: [],
        transitions: [],
        errors: [],
        warnings: [],
        infos: [],
        format: 'unknown'
    };
}

/* ═══ HELPER: FIND OR CREATE STATE ═══ */

/**
 * Find or create a state in the result.
 * @param {object} result - Parse result object.
 * @param {string} name - State name.
 * @param {string} type - State type.
 * @returns {object} The state object.
 */
function getOrAddState(result, name, type) {
    var s = result.states.find(function(x) { return x.name === name; });
    if (!s) {
        s = {
            name: name,
            type: type || 'normal',
            onEntry: '',
            onExit: '',
            entryGuard: '',
            exitGuard: '',
            internalEvents: [],
            comment: '',
            history: 'none'
        };
        result.states.push(s);
    } else if (type && type !== 'normal') {
        s.type = type;
    }
    return s;
}

/* ═══ STRIP COMMENTS ═══ */

/**
 * Remove C-style block comments from code.
 * Line comments are preserved as they may contain markers.
 * @param {string} src - Source text.
 * @returns {string} Source without block comments.
 */
function stripBlockComments(src) {
    return src.replace(/\/\*[\s\S]*?\*\//g, ' ');
}

/* ═══ HEADER PARSER ═══ */

/**
 * Parse the header section to extract class name, state enum, event enum,
 * and initial state from constructor.
 * @param {string} header - Header content.
 * @param {object} result - Parse result to populate.
 */
function parseHeader(header, result) {
    if (!header) {
        return;
    }
    var clean = stripBlockComments(header);

    /* Class name */
    var clsMatch = clean.match(/class\s+(\w+)/);
    if (clsMatch) {
        result.name = clsMatch[1];
        result.infos.push('Found class: ' + clsMatch[1]);
    }

    /* State enum */
    var stateEnumRx = /enum\s+(?:class\s+)?State\s*(?::\s*\w+)?\s*\{([^}]+)\}/;
    var sem = stateEnumRx.exec(clean);
    if (sem) {
        sem[1].split(',').forEach(function(part) {
            var name = part.replace(/=\s*\d+/, '').trim();
            if (name && !/^\s*$/.test(name)) {
                getOrAddState(result, name, 'normal');
                result.infos.push('State: ' + name);
            }
        });
    }

    /* Event enum */
    var eventEnumRx = /enum\s+(?:class\s+)?Event\s*(?::\s*\w+)?\s*\{([^}]+)\}/;
    var eem = eventEnumRx.exec(clean);
    if (eem) {
        result.infos.push('Found event enum');
    }

    /* Initial state from constructor initializer */
    var ctorRx = /state_\s*\(\s*State::([\w]+)\s*\)/;
    var cm = ctorRx.exec(clean);
    if (cm) {
        result.initial = cm[1];
        getOrAddState(result, cm[1], 'initial');
        result.infos.push('Initial state: ' + cm[1]);
    }

    /* Initial state from assignment */
    if (!result.initial) {
        var assignRx = /state_\s*=\s*State::([\w]+)/;
        var am = assignRx.exec(clean);
        if (am) {
            result.initial = am[1];
            getOrAddState(result, am[1], 'initial');
            result.infos.push('Initial state (assignment): ' + am[1]);
        }
    }

    /* Final states from isFinal — do not override initial type */
    var finalRx = /isFinal[^{]*\{[^}]*return\s+([^;]+);/;
    var fm = finalRx.exec(clean);
    if (fm) {
        var expr = fm[1];
        result.states.forEach(function(s) {
            if (expr.indexOf(s.name) !== -1 && s.type !== 'initial') {
                s.type = 'final';
                result.infos.push('Final state: ' + s.name);
            }
        });
    }
}

/* ═══ SOURCE PARSER ═══ */

/**
 * Parse the source section to extract transitions and entry/exit actions.
 * @param {string} source - Source content.
 * @param {object} result - Parse result to populate.
 */
function parseSource(source, result) {
    if (!source) {
        return;
    }
    var clean = stripBlockComments(source);

    /* Transitions from switch/case + if blocks */
    var caseRx = /case\s+State::(\w+)\s*:([^]*?)(?=case\s+State::|default\s*:|$)/g;
    var cm;
    while ((cm = caseRx.exec(clean)) !== null) {
        var fromState = cm[1];
        getOrAddState(result, fromState);
        var block = cm[2];

        /* if (event == Event::NAME) { ... state_ = State::TO ... } */
        var ifRx = /if\s*\(\s*event\s*==\s*Event::(\w+)\s*\)\s*\{([^}]*)\}/g;
        var im;
        while ((im = ifRx.exec(block)) !== null) {
            var evName = im[1];
            var ifBody = im[2];

            var tgtRx = /state_\s*=\s*State::(\w+)\s*;/;
            var tgtM = tgtRx.exec(ifBody);
            if (tgtM) {
                var toState = tgtM[1];
                getOrAddState(result, toState);

                /* Extract guard */
                var guardRx = /if\s*\(\s*([^)]+)\s*\)/;
                var gm = ifBody.match(guardRx);
                var guard = gm ? gm[1].trim() : '';

                /* Extract action: lines that aren't state assignment */
                var actionLines = ifBody.split(';')
                    .map(function(l) { return l.trim(); })
                    .filter(function(l) {
                        return l && l.length > 0 &&
                            !tgtRx.test(l + ';') &&
                            !/^\s*$/.test(l) &&
                            !guardRx.test(l);
                    });
                var action = actionLines.join('; ');

                result.transitions.push({
                    from: fromState,
                    to: toState,
                    event: evName,
                    guard: guard,
                    action: action
                });
                result.infos.push('Trans: ' + fromState + ' -> ' + toState + ' [' + evName + ']');
            }
        }
    }

    /* Entry actions from onEntry function */
    var entryFnRx = /void\s+\w*onEntry\w*\s*\([^)]*\)\s*\{([^]*?)(?=\nvoid\s|\n\s*\}\s*$|\Z)/;
    var efm = entryFnRx.exec(clean);
    if (efm) {
        parseCaseActions(efm[1], result, 'onEntry');
    }

    /* Exit actions from onExit function */
    var exitFnRx = /void\s+\w*onExit\w*\s*\([^)]*\)\s*\{([^]*?)(?=\nvoid\s|\n\s*\}\s*$|\Z)/;
    var xfm = exitFnRx.exec(clean);
    if (xfm) {
        parseCaseActions(xfm[1], result, 'onExit');
    }
}

/**
 * Parse case blocks for entry/exit actions.
 * @param {string} body - Switch body content.
 * @param {object} result - Parse result.
 * @param {string} field - 'onEntry' or 'onExit'.
 */
function parseCaseActions(body, result, field) {
    var caseRx = /case\s+State::(\w+)\s*:([^]*?)(?=case\s+State::|default\s*:|$)/g;
    var cm;
    while ((cm = caseRx.exec(body)) !== null) {
        var sName = cm[1];
        var s = result.states.find(function(x) { return x.name === sName; });
        if (s) {
            var actions = cm[2]
                .replace(/break\s*;/g, '')
                .split(';')
                .map(function(l) { return l.trim(); })
                .filter(function(l) { return l && !/^\s*$/.test(l); });
            if (actions.length > 0) {
                s[field] = actions.join(';\n');
            }
        }
    }
}

/* ═══ DSL PARSER ═══ */

/**
 * Parse simple DSL format.
 * @param {string} src - DSL source text.
 * @returns {object} Parse result.
 */
function parseDsl(src) {
    var r = blankResult();
    r.format = 'dsl';
    var lines = src.split('\n');

    lines.forEach(function(raw, i) {
        var l = raw.trim();
        if (!l || l.startsWith('//') || l.startsWith('/*') || l.startsWith('*')) {
            return;
        }
        var ln = i + 1;

        if (/^FSM\s*:/i.test(l)) {
            r.name = l.replace(/^FSM\s*:\s*/i, '').trim();
            r.infos.push('FSM name: "' + r.name + '"');
        }
        else if (/^STATE\s*:/i.test(l)) {
            var name = l.replace(/^STATE\s*:\s*/i, '').trim();
            if (name) {
                getOrAddState(r, name, 'normal');
                r.infos.push('State: "' + name + '"');
            }
        }
        else if (/^INIT\s*:/i.test(l)) {
            var initName = l.replace(/^INIT\s*:\s*/i, '').trim();
            getOrAddState(r, initName, 'initial');
            r.initial = initName;
            r.infos.push('Initial: "' + initName + '"');
        }
        else if (/^FINAL\s*:/i.test(l)) {
            var finalName = l.replace(/^FINAL\s*:\s*/i, '').trim();
            getOrAddState(r, finalName, 'final');
            r.infos.push('Final: "' + finalName + '"');
        }
        else if (/^TRANS\s*:/i.test(l)) {
            var m = l.match(
                /^TRANS\s*:\s*(\w+)\s*->\s*(\w+)\s*:\s*([\w]+)(?:\s*\[([^\]]*)\])?(?:\s*\/\s*(.+))?/i
            );
            if (m) {
                getOrAddState(r, m[1]);
                getOrAddState(r, m[2]);
                r.transitions.push({
                    from: m[1],
                    to: m[2],
                    event: m[3].trim(),
                    guard: (m[4] || '').trim(),
                    action: (m[5] || '').trim()
                });
                r.infos.push('Trans: ' + m[1] + ' -> ' + m[2] + ' [' + m[3] + ']');
            } else {
                r.errors.push('Line ' + ln + ': Bad TRANS syntax: "' + l + '"');
            }
        }
        else if (/^ON_ENTRY\s*:/i.test(l)) {
            var em = l.match(/^ON_ENTRY\s*:\s*(\w+)\s*\/\s*(.+)/i);
            if (em) {
                var es = getOrAddState(r, em[1]);
                es.onEntry = em[2].trim();
            }
        }
        else if (/^ON_EXIT\s*:/i.test(l)) {
            var xm = l.match(/^ON_EXIT\s*:\s*(\w+)\s*\/\s*(.+)/i);
            if (xm) {
                var xs = getOrAddState(r, xm[1]);
                xs.onExit = xm[2].trim();
            }
        }
        else if (/^INTERNAL\s*:/i.test(l)) {
            var intm = l.match(
                /^INTERNAL\s*:\s*(\w+)\s*\/\s*(\w+)(?:\s*\[([^\]]*)\])?(?:\s*\/\s*(.+))?/i
            );
            if (intm) {
                var ints = getOrAddState(r, intm[1]);
                ints.internalEvents.push({
                    event: intm[2].trim(),
                    guard: (intm[3] || '').trim(),
                    action: (intm[4] || '').trim()
                });
            }
        }
        else if (/^COMMENT\s*:/i.test(l)) {
            var cmm = l.match(/^COMMENT\s*:\s*(\w+)\s*\/\s*(.+)/i);
            if (cmm) {
                var cs = getOrAddState(r, cmm[1]);
                cs.comment = cmm[2].trim();
            }
        }
    });

    /* Cross-check */
    r.transitions.forEach(function(t) {
        if (!r.states.find(function(s) { return s.name === t.from; })) {
            r.errors.push('Transition from unknown state: "' + t.from + '"');
        }
        if (!r.states.find(function(s) { return s.name === t.to; })) {
            r.errors.push('Transition to unknown state: "' + t.to + '"');
        }
    });
    if (!r.initial && r.states.length > 0) {
        r.warnings.push('No INIT state defined');
    }
    return r;
}

/* ═══ MARKED FORMAT PARSER ═══ */

/**
 * Parse source with FSM_HEADER / FSM_SOURCE section markers.
 * @param {string} src - Source text with markers.
 * @returns {object} Parse result.
 */
function parseMarked(src) {
    var r = blankResult();
    r.format = 'marked';

    var header = extractSection(src, MARKER_HEADER_BEGIN, MARKER_HEADER_END);
    var source = extractSection(src, MARKER_SOURCE_BEGIN, MARKER_SOURCE_END);

    if (!header && !source) {
        r.errors.push('No FSM_HEADER or FSM_SOURCE sections found');
        return r;
    }

    if (header) {
        r.infos.push('Found header section');
        parseHeader(header, r);
    }
    if (source) {
        r.infos.push('Found source section');
        parseSource(source, r);
    }

    if (r.states.length === 0) {
        r.errors.push('No states found in parsed code');
    }
    return r;
}

/* ═══ MASTER PARSER ═══ */

/**
 * Parse input text — auto-detects format.
 * @param {string} src - Input text.
 * @returns {object} Parse result with states, transitions, errors, etc.
 */
function parseInput(src) {
    if (typeof src !== 'string' || src.trim().length === 0) {
        var r = blankResult();
        r.errors.push('Input is empty');
        return r;
    }

    var fmt = detectFormat(src);
    switch (fmt) {
        case 'marked':
            return parseMarked(src);
        case 'dsl':
            return parseDsl(src);
        default:
            /* Try DSL as last resort */
            var result = parseDsl(src);
            if (result.states.length === 0) {
                result.errors.push('Could not detect input format');
            }
            return result;
    }
}

/* ═══ APPLY PARSE RESULT TO FSM MODEL ═══ */

/**
 * Apply parse result to the global FSM model.
 * This function is browser-only (needs DOM + model globals).
 * @param {object} result - Parse result from parseInput().
 */
function applyParseResult(result) {
    /* These are global functions from model.js (browser context) */
    if (typeof snap === 'function') { snap(); }

    fsm.states = [];
    fsm.transitions = [];
    uid = 1;
    fsm.name = result.name || 'MyFSM';

    var nameInput = (typeof document !== 'undefined') ?
        document.getElementById('fsm-name') : null;
    if (nameInput) { nameInput.value = fsm.name; }

    var cw = (typeof document !== 'undefined') ?
        document.getElementById('cw') : null;
    var cwRect = cw ? cw.getBoundingClientRect() : { width: 800, height: 600 };
    var cx = cwRect.width / 2;
    var cy = cwRect.height / 2;
    var count = result.states.length;
    var R = Math.max(140, Math.min(240, 62 * count));
    var idMap = {};

    result.states.forEach(function(s, i) {
        var a = (2 * Math.PI * i / count) - Math.PI / 2;
        var id = nid();
        idMap[s.name] = id;
        var panXVal = (typeof panX !== 'undefined') ? panX : 0;
        var panYVal = (typeof panY !== 'undefined') ? panY : 0;
        var zoomVal = (typeof zoom !== 'undefined') ? zoom : 1;

        fsm.states.push({
            id: id,
            name: s.name,
            type: s.type,
            x: (cx + R * Math.cos(a) - panXVal) / zoomVal,
            y: (cy + R * Math.sin(a) - panYVal) / zoomVal,
            onEntry: s.onEntry || '',
            onExit: s.onExit || '',
            entryGuard: s.entryGuard || '',
            exitGuard: s.exitGuard || '',
            internalEvents: s.internalEvents || [],
            comment: s.comment || '',
            history: s.history || 'none'
        });
    });

    if (result.initial && idMap[result.initial]) {
        fsm.initial = idMap[result.initial];
    }

    result.transitions.forEach(function(t) {
        if (idMap[t.from] && idMap[t.to]) {
            fsm.transitions.push({
                id: nid(),
                from: idMap[t.from],
                to: idMap[t.to],
                event: t.event || '',
                guard: t.guard || '',
                action: t.action || ''
            });
        }
    });

    if (typeof renderAll === 'function') { renderAll(); }
}

/* ═══ GENERATE DIAGRAM FROM INPUT (new) ═══ */

/**
 * Parse the code textarea and build the diagram.
 * Browser-only function.
 */
function parseAndBuild() {
    var codeEl = (typeof document !== 'undefined') ?
        document.getElementById('code-ta') : null;
    if (!codeEl) { return; }

    var code = codeEl.value;
    if (!code.trim()) {
        if (typeof toast === 'function') { toast('Editor is empty', 'err'); }
        return;
    }

    var result = parseInput(code);
    if (result.states.length === 0) {
        var hint = 'No states found. ';
        if (result.format === 'unknown') {
            hint += 'Use DSL format or marked C++ format.';
        }
        if (typeof toast === 'function') { toast(hint, 'err'); }
        return;
    }

    applyParseResult(result);
    if (typeof toast === 'function') {
        toast(
            'Parsed (' + result.format.toUpperCase() + '): ' +
            fsm.states.length + ' states, ' +
            fsm.transitions.length + ' transitions',
            'ok'
        );
    }
}

/* ═══ NODE.JS EXPORTS (for Jest testing) ═══ */
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        MARKER_HEADER_BEGIN: MARKER_HEADER_BEGIN,
        MARKER_HEADER_END: MARKER_HEADER_END,
        MARKER_SOURCE_BEGIN: MARKER_SOURCE_BEGIN,
        MARKER_SOURCE_END: MARKER_SOURCE_END,
        extractSection: extractSection,
        detectFormat: detectFormat,
        blankResult: blankResult,
        getOrAddState: getOrAddState,
        stripBlockComments: stripBlockComments,
        parseHeader: parseHeader,
        parseSource: parseSource,
        parseDsl: parseDsl,
        parseMarked: parseMarked,
        parseInput: parseInput
    };
}
