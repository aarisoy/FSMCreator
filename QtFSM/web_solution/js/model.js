// model.js — FSM config-based data model + undo/redo
//
// THE single source of truth. All other modules read `fsm`.
// Always call snap() BEFORE changing fsm so undo works.
//
// Config-based FSM structure:
// fsm = {
//   name: string,
//   namespace: string,
//   description: string,
//   initial: string (state id),
//   states: [{ id, name, type, x, y, onEntry, onExit, entryGuard, exitGuard,
//              internalEvents: [{ event, guard, action }], comment, history }],
//   transitions: [{ id, from, to, event, guard, action }]
// }

/* ═══ DATA ═══ */
let fsm = { name: 'MyFSM', initial: '', states: [], transitions: [] };
let hist = [];
let redos = [];
let uid = 1;

/**
 * Generate a unique node ID.
 * @returns {string} Unique ID string like "n1", "n2", etc.
 */
function nid() {
    return 'n' + (uid++);
}

/**
 * Create a default state config object.
 * @param {string} name - State name.
 * @param {string} type - State type: "initial", "normal", or "final".
 * @returns {object} State config object.
 */
function createStateConfig(name, type) {
    if (typeof name !== 'string' || name.trim().length === 0) {
        return null;
    }
    const validTypes = ['initial', 'normal', 'final'];
    const safeType = validTypes.includes(type) ? type : 'normal';

    return {
        id: nid(),
        name: name.trim(),
        type: safeType,
        x: 0,
        y: 0,
        onEntry: '',
        onExit: '',
        entryGuard: '',
        exitGuard: '',
        internalEvents: [],
        comment: '',
        history: 'none'
    };
}

/**
 * Create a default transition config object.
 * @param {string} from - Source state ID.
 * @param {string} to - Target state ID.
 * @param {string} event - Event name.
 * @param {string} guard - Guard condition (optional).
 * @param {string} action - Action to execute (optional).
 * @returns {object|null} Transition config object, or null if invalid.
 */
function createTransitionConfig(from, to, event, guard, action) {
    if (typeof from !== 'string' || from.trim().length === 0) {
        return null;
    }
    if (typeof to !== 'string' || to.trim().length === 0) {
        return null;
    }

    return {
        id: nid(),
        from: from,
        to: to,
        event: (typeof event === 'string') ? event.trim() : '',
        guard: (typeof guard === 'string') ? guard.trim() : '',
        action: (typeof action === 'string') ? action.trim() : ''
    };
}

/**
 * Add a state to the FSM. Prevents duplicate names.
 * @param {string} name - State name.
 * @param {string} type - State type.
 * @returns {object|null} The added state, or null if name is duplicate/invalid.
 */
function addState(name, type) {
    if (typeof name !== 'string' || name.trim().length === 0) {
        return null;
    }
    const trimmedName = name.trim();
    const duplicate = fsm.states.find(function(s) {
        return s.name === trimmedName;
    });
    if (duplicate) {
        return null;
    }
    const state = createStateConfig(trimmedName, type);
    if (state === null) {
        return null;
    }
    fsm.states.push(state);
    if (state.type === 'initial' && !fsm.initial) {
        fsm.initial = state.id;
    }
    return state;
}

/**
 * Remove a state by ID and any associated transitions.
 * @param {string} stateId - The state ID to remove.
 * @returns {boolean} True if state was found and removed.
 */
function removeState(stateId) {
    const idx = fsm.states.findIndex(function(s) {
        return s.id === stateId;
    });
    if (idx === -1) {
        return false;
    }
    fsm.states.splice(idx, 1);
    fsm.transitions = fsm.transitions.filter(function(t) {
        return t.from !== stateId && t.to !== stateId;
    });
    if (fsm.initial === stateId) {
        fsm.initial = '';
    }
    return true;
}

/**
 * Add a transition to the FSM.
 * @param {string} from - Source state ID.
 * @param {string} to - Target state ID.
 * @param {string} event - Event name.
 * @param {string} guard - Guard condition (optional).
 * @param {string} action - Action (optional).
 * @returns {object|null} The added transition, or null if invalid.
 */
function addTransition(from, to, event, guard, action) {
    const fromState = fsm.states.find(function(s) { return s.id === from; });
    const toState = fsm.states.find(function(s) { return s.id === to; });
    if (!fromState || !toState) {
        return null;
    }
    const trans = createTransitionConfig(from, to, event, guard, action);
    if (trans === null) {
        return null;
    }
    fsm.transitions.push(trans);
    return trans;
}

/**
 * Remove a transition by ID.
 * @param {string} transId - The transition ID to remove.
 * @returns {boolean} True if transition was found and removed.
 */
function removeTransition(transId) {
    const idx = fsm.transitions.findIndex(function(t) {
        return t.id === transId;
    });
    if (idx === -1) {
        return false;
    }
    fsm.transitions.splice(idx, 1);
    return true;
}

/**
 * Reset the FSM to an empty state.
 */
function resetFsm() {
    fsm.name = 'MyFSM';
    fsm.initial = '';
    fsm.states = [];
    fsm.transitions = [];
    hist = [];
    redos = [];
    uid = 1;
}

/**
 * Take a snapshot before modifying (for undo).
 */
function snap() {
    hist.push(JSON.stringify(fsm));
    redos = [];
    if (hist.length > 80) {
        hist.shift();
    }
    if (typeof updUR === 'function') {
        updUR();
    }
}

/**
 * Undo the last change.
 * @returns {boolean} True if undo was performed.
 */
function undo() {
    if (!hist.length) {
        return false;
    }
    redos.push(JSON.stringify(fsm));
    fsm = JSON.parse(hist.pop());
    if (typeof updUR === 'function') {
        updUR();
    }
    if (typeof renderAll === 'function') {
        renderAll();
    }
    return true;
}

/**
 * Redo the last undone change.
 * @returns {boolean} True if redo was performed.
 */
function redo() {
    if (!redos.length) {
        return false;
    }
    hist.push(JSON.stringify(fsm));
    fsm = JSON.parse(redos.pop());
    if (typeof updUR === 'function') {
        updUR();
    }
    if (typeof renderAll === 'function') {
        renderAll();
    }
    return true;
}

/**
 * Update undo/redo button states (browser only).
 */
function updUR() {
    var undoBtn = (typeof document !== 'undefined') ?
        document.getElementById('btn-undo') : null;
    var redoBtn = (typeof document !== 'undefined') ?
        document.getElementById('btn-redo') : null;
    if (undoBtn) { undoBtn.disabled = !hist.length; }
    if (redoBtn) { redoBtn.disabled = !redos.length; }
}

/**
 * Get undo history length.
 * @returns {number}
 */
function getHistoryLength() {
    return hist.length;
}

/**
 * Get redo history length.
 * @returns {number}
 */
function getRedoLength() {
    return redos.length;
}

/* ═══ NODE.JS EXPORTS (for Jest testing) ═══ */
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        getFsm: function() { return fsm; },
        setFsm: function(f) { fsm = f; },
        nid: nid,
        createStateConfig: createStateConfig,
        createTransitionConfig: createTransitionConfig,
        addState: addState,
        removeState: removeState,
        addTransition: addTransition,
        removeTransition: removeTransition,
        resetFsm: resetFsm,
        snap: snap,
        undo: undo,
        redo: redo,
        getHistoryLength: getHistoryLength,
        getRedoLength: getRedoLength
    };
}
