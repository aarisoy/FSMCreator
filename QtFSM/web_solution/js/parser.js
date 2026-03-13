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
 * Detect the State-like and Event-like enum type prefix from code.
 * Matches: State, StateId, BootState, ProtocolState, etc.
 * @param {string} code - Code to examine.
 * @returns {object} Object with statePrefix and eventPrefix strings.
 */
function detectEnumPrefixes(code) {
    var statePrefix = 'State';
    var eventPrefix = 'Event';

    /* Match enum class <prefix> where prefix starts with or contains 'State' */
    var spRx = /enum\s+class\s+(\w*State\w*)\s*(?::\s*\w+)?\s*\{/;
    var spMatch = spRx.exec(code);
    if (spMatch) {
        statePrefix = spMatch[1];
    }

    /* Match enum class <prefix> where prefix starts with or contains 'Event' */
    var epRx = /enum\s+class\s+(\w*Event\w*)\s*(?::\s*\w+)?\s*\{/;
    var epMatch = epRx.exec(code);
    if (epMatch) {
        eventPrefix = epMatch[1];
    }

    return { statePrefix: statePrefix, eventPrefix: eventPrefix };
}

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

    /* Config-format FSM name from comment (takes priority over class name) */
    var cfgNameRx = /\/\/\s*Auto-generated FSM Config\s*-\s*(.+)/;
    var cfgNameMatch = cfgNameRx.exec(clean);
    if (cfgNameMatch) {
        result.name = cfgNameMatch[1].trim();
        result.infos.push('Config FSM name: ' + result.name);
    }

    /* Detect state/event enum prefixes (State, StateId, BootState, etc.) */
    var prefixes = detectEnumPrefixes(clean);
    result._statePrefix = prefixes.statePrefix;
    result._eventPrefix = prefixes.eventPrefix;

    /* State enum — flexible prefix */
    var stateEnumRx = new RegExp(
        'enum\\s+(?:class\\s+)?' + escapeRegex(prefixes.statePrefix) +
        '\\s*(?::\\s*\\w+)?\\s*\\{([^}]+)\\}'
    );
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

    /* Event enum — flexible prefix */
    var eventEnumRx = new RegExp(
        'enum\\s+(?:class\\s+)?' + escapeRegex(prefixes.eventPrefix) +
        '\\s*(?::\\s*\\w+)?\\s*\\{([^}]+)\\}'
    );
    var eem = eventEnumRx.exec(clean);
    if (eem) {
        result.infos.push('Found event enum');
    }

    /* Initial state from constructor initializer — flexible prefix */
    var ctorRx = new RegExp(
        'state_\\s*\\(\\s*' + escapeRegex(prefixes.statePrefix) + '::(\\w+)\\s*\\)'
    );
    var cm = ctorRx.exec(clean);
    if (cm) {
        result.initial = cm[1];
        getOrAddState(result, cm[1], 'initial');
        result.infos.push('Initial state: ' + cm[1]);
    }

    /* Initial state from assignment — flexible prefix */
    if (!result.initial) {
        var assignRx = new RegExp(
            'state_\\s*=\\s*' + escapeRegex(prefixes.statePrefix) + '::(\\w+)'
        );
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

/**
 * Escape a string for safe use in a RegExp constructor.
 * @param {string} str - String to escape.
 * @returns {string} Escaped string.
 */
function escapeRegex(str) {
    if (typeof str !== 'string') {
        return '';
    }
    return str.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

/* ═══ SOURCE PARSER ═══ */

/**
 * Parse the source section to extract transitions and entry/exit actions.
 * Supports both switch/case patterns and config-map patterns.
 * @param {string} source - Source content.
 * @param {object} result - Parse result to populate.
 */
function parseSource(source, result) {
    if (!source) {
        return;
    }
    var clean = stripBlockComments(source);
    var sp = result._statePrefix || 'State';
    var ep = result._eventPrefix || 'Event';

    /* Try switch/case pattern first — flexible prefix */
    var caseRxStr = 'case\\s+' + escapeRegex(sp) + '::(\\w+)\\s*:([^]*?)(?=case\\s+' +
                    escapeRegex(sp) + '::|default\\s*:|$)';
    var caseRx = new RegExp(caseRxStr, 'g');
    var cm;
    var foundSwitchCase = false;
    while ((cm = caseRx.exec(clean)) !== null) {
        foundSwitchCase = true;
        var fromState = cm[1];
        getOrAddState(result, fromState);
        var block = cm[2];

        /* if (event == Event::NAME) { ... state_ = State::TO ... } */
        var ifRxStr = 'if\\s*\\(\\s*event\\s*==\\s*' + escapeRegex(ep) +
                      '::(\\w+)\\s*\\)\\s*\\{([^}]*)\\}';
        var ifRx = new RegExp(ifRxStr, 'g');
        var im;
        while ((im = ifRx.exec(block)) !== null) {
            var evName = im[1];
            var ifBody = im[2];

            var tgtRxStr = 'state_\\s*=\\s*' + escapeRegex(sp) + '::(\\w+)\\s*;';
            var tgtRx = new RegExp(tgtRxStr);
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

    /* Config-map pattern: cfg[StatePrefix::FROM] = { ..., { {EventPrefix::EV, StatePrefix::TO, ...}, } } */
    if (!foundSwitchCase) {
        parseConfigMapTransitions(clean, result, sp, ep);
    }

    /* QtFSM config format: cfg.states["S1"] = StateConfig{ "Name", ..., { {"ev", "tgt", "guard", "action"}, } } */
    if (!foundSwitchCase && result.transitions.length === 0) {
        parseQtFsmConfig(clean, result);
    }

    /* Entry actions from onEntry function — flexible prefix */
    var entryFnRx = /void\s+\w*onEntry\w*\s*\([^)]*\)\s*\{([^]*?)(?=\nvoid\s|\n\s*\}\s*$|\Z)/;
    var efm = entryFnRx.exec(clean);
    if (efm) {
        parseCaseActions(efm[1], result, 'onEntry', sp);
    }

    /* Exit actions from onExit function — flexible prefix */
    var exitFnRx = /void\s+\w*onExit\w*\s*\([^)]*\)\s*\{([^]*?)(?=\nvoid\s|\n\s*\}\s*$|\Z)/;
    var xfm = exitFnRx.exec(clean);
    if (xfm) {
        parseCaseActions(xfm[1], result, 'onExit', sp);
    }
}

/**
 * Parse config-map transition patterns.
 * Matches: cfg[StatePrefix::FROM] = { StatePrefix::FROM, "FROM", ..., { {EventPrefix::EV, StatePrefix::TO, ...}, ... } }
 * @param {string} code - Cleaned source code.
 * @param {object} result - Parse result.
 * @param {string} sp - State enum prefix.
 * @param {string} ep - Event enum prefix.
 */
function parseConfigMapTransitions(code, result, sp, ep) {
    /* Match config assignments: variable[StatePrefix::NAME] = { ... }; or without trailing semicolon */
    var cfgEntryRxStr = '\\w+\\[' + escapeRegex(sp) + '::(\\w+)\\]\\s*=\\s*\\{';
    var cfgEntryRx = new RegExp(cfgEntryRxStr, 'g');
    var em;
    while ((em = cfgEntryRx.exec(code)) !== null) {
        var fromState = em[1];
        getOrAddState(result, fromState);

        /* Find the block following this match — we need to handle nested braces */
        var startIdx = em.index + em[0].length;
        var cfgBlock = extractBalancedBraces(code, startIdx - 1);
        if (!cfgBlock) {
            continue;
        }

        /* Extract transitions: {EventPrefix::EV, StatePrefix::TO, ...} */
        var transRxStr = '\\{\\s*' + escapeRegex(ep) + '::(\\w+)\\s*,\\s*' +
                         escapeRegex(sp) + '::(\\w+)';
        var transRx = new RegExp(transRxStr, 'g');
        var tm;
        while ((tm = transRx.exec(cfgBlock)) !== null) {
            var evName = tm[1];
            var toState = tm[2];
            getOrAddState(result, toState);

            result.transitions.push({
                from: fromState,
                to: toState,
                event: evName,
                guard: '',
                action: ''
            });
            result.infos.push('Config trans: ' + fromState + ' -> ' + toState + ' [' + evName + ']');
        }
    }
}

/**
 * Parse QtFSM config format: cfg.states["ID"] = StateConfig{ "Name", x, y, isFinal, "entry", "exit", { transitions } }
 * @param {string} code - Cleaned source code.
 * @param {object} result - Parse result.
 */
function parseQtFsmConfig(code, result) {
    /* Detect FSM name from comment */
    var nameRx = /\/\/\s*Auto-generated FSM Config\s*-\s*(.+)/;
    var nameMatch = nameRx.exec(code);
    if (nameMatch && !result.name) {
        result.name = nameMatch[1].trim();
    }

    /* Detect initial state */
    var initialRx = /cfg\.initial\s*=\s*"([^"]*)"/;
    var initialMatch = initialRx.exec(code);
    var initialId = initialMatch ? initialMatch[1] : null;

    /* Parse state configs */
    var stateRx = /cfg\.states\["([^"]*)"\]\s*=\s*StateConfig\s*\{/g;
    var sm;
    var stateIdToName = {};
    while ((sm = stateRx.exec(code)) !== null) {
        var stateId = sm[1];

        /* Extract the block */
        var blockStart = sm.index + sm[0].length;
        var cfgBlock = extractBalancedBraces(code, blockStart - 1);
        if (!cfgBlock) {
            continue;
        }

        /* First quoted string is the state name */
        var nameMatch2 = cfgBlock.match(/^\s*"([^"]*)"/);
        var stateName = nameMatch2 ? nameMatch2[1] : stateId;
        stateIdToName[stateId] = stateName;

        getOrAddState(result, stateName, 'normal');

        /* Check isFinal (4th field) */
        var fieldsBeforeTrans = cfgBlock.split('{')[0];
        if (/true/.test(fieldsBeforeTrans) && fieldsBeforeTrans.split(',').length > 3) {
            var s = result.states.find(function(x) { return x.name === stateName; });
            if (s && s.type !== 'initial') {
                s.type = 'final';
            }
        }

        /* Extract entry/exit actions (5th and 6th quoted strings in the main block) */
        var quotedStrings = fieldsBeforeTrans.match(/"([^"]*)"/g);
        if (quotedStrings && quotedStrings.length >= 3) {
            var entryAction = quotedStrings[1].replace(/^"|"$/g, '');
            var exitAction = quotedStrings[2].replace(/^"|"$/g, '');
            var stObj = result.states.find(function(x) { return x.name === stateName; });
            if (stObj) {
                if (entryAction) { stObj.onEntry = entryAction; }
                if (exitAction) { stObj.onExit = exitAction; }
            }
        }

        /* Parse transitions: {"event", "target", "guard", "action"} */
        var transRx = /\{\s*"([^"]*)"\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"\s*\}/g;
        var tm;
        while ((tm = transRx.exec(cfgBlock)) !== null) {
            result.transitions.push({
                from: stateName,
                to: tm[2],  /* Will be mapped from ID to name later */
                event: tm[1],
                guard: tm[3],
                action: tm[4]
            });
            result.infos.push('QtFSM trans: ' + stateName + ' -> ' + tm[2] + ' [' + tm[1] + ']');
        }
    }

    /* Map transition targets from IDs to state names */
    result.transitions.forEach(function(t) {
        if (stateIdToName[t.to]) {
            t.to = stateIdToName[t.to];
        }
        if (stateIdToName[t.from]) {
            t.from = stateIdToName[t.from];
        }
    });

    /* Set initial state */
    if (initialId && stateIdToName[initialId]) {
        result.initial = stateIdToName[initialId];
        getOrAddState(result, stateIdToName[initialId], 'initial');
    } else if (initialId) {
        result.initial = initialId;
    }

    /* Ensure referenced states exist */
    result.transitions.forEach(function(t) {
        getOrAddState(result, t.to);
        getOrAddState(result, t.from);
    });
}

/**
 * Extract content within balanced braces starting at the given opening brace.
 * @param {string} code - Full code string.
 * @param {number} openIdx - Index of the opening '{' character.
 * @returns {string|null} Content between braces (exclusive), or null if unbalanced.
 */
function extractBalancedBraces(code, openIdx) {
    if (code[openIdx] !== '{') {
        return null;
    }
    var depth = 1;
    var i = openIdx + 1;
    while (i < code.length && depth > 0) {
        if (code[i] === '{') {
            depth++;
        } else if (code[i] === '}') {
            depth--;
        }
        i++;
    }
    if (depth !== 0) {
        return null;
    }
    return code.substring(openIdx + 1, i - 1);
}

/**
 * Parse case blocks for entry/exit actions.
 * @param {string} body - Switch body content.
 * @param {object} result - Parse result.
 * @param {string} field - 'onEntry' or 'onExit'.
 * @param {string} sp - State enum prefix (optional, defaults to 'State').
 */
function parseCaseActions(body, result, field, sp) {
    var prefix = sp || 'State';
    var caseRxStr = 'case\\s+' + escapeRegex(prefix) + '::(\\w+)\\s*:([^]*?)(?=case\\s+' +
                    escapeRegex(prefix) + '::|default\\s*:|$)';
    var caseRx = new RegExp(caseRxStr, 'g');
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
        detectEnumPrefixes: detectEnumPrefixes,
        escapeRegex: escapeRegex,
        extractBalancedBraces: extractBalancedBraces,
        parseHeader: parseHeader,
        parseSource: parseSource,
        parseConfigMapTransitions: parseConfigMapTransitions,
        parseQtFsmConfig: parseQtFsmConfig,
        parseDsl: parseDsl,
        parseMarked: parseMarked,
        parseInput: parseInput
    };
}
