// model.test.js — Jest tests for FSM config-based data model
'use strict';

const model = require('../js/model.js');

describe('Model Module', function() {

    beforeEach(function() {
        model.resetFsm();
    });

    describe('createStateConfig', function() {
        test('creates a valid state config with defaults', function() {
            var state = model.createStateConfig('Idle', 'normal');
            expect(state).not.toBeNull();
            expect(state.name).toBe('Idle');
            expect(state.type).toBe('normal');
            expect(state.onEntry).toBe('');
            expect(state.onExit).toBe('');
            expect(state.entryGuard).toBe('');
            expect(state.exitGuard).toBe('');
            expect(state.internalEvents).toEqual([]);
            expect(state.comment).toBe('');
            expect(state.history).toBe('none');
            expect(typeof state.id).toBe('string');
        });

        test('creates initial state type', function() {
            var state = model.createStateConfig('Start', 'initial');
            expect(state.type).toBe('initial');
        });

        test('creates final state type', function() {
            var state = model.createStateConfig('End', 'final');
            expect(state.type).toBe('final');
        });

        test('defaults to normal for invalid type', function() {
            var state = model.createStateConfig('S1', 'bogus');
            expect(state.type).toBe('normal');
        });

        test('returns null for empty name', function() {
            expect(model.createStateConfig('', 'normal')).toBeNull();
        });

        test('returns null for whitespace-only name', function() {
            expect(model.createStateConfig('   ', 'normal')).toBeNull();
        });

        test('returns null for non-string name', function() {
            expect(model.createStateConfig(123, 'normal')).toBeNull();
            expect(model.createStateConfig(null, 'normal')).toBeNull();
            expect(model.createStateConfig(undefined, 'normal')).toBeNull();
        });

        test('trims whitespace from name', function() {
            var state = model.createStateConfig('  Idle  ', 'normal');
            expect(state.name).toBe('Idle');
        });
    });

    describe('createTransitionConfig', function() {
        test('creates a valid transition config', function() {
            var trans = model.createTransitionConfig('n1', 'n2', 'START', 'x > 0', 'onStart()');
            expect(trans).not.toBeNull();
            expect(trans.from).toBe('n1');
            expect(trans.to).toBe('n2');
            expect(trans.event).toBe('START');
            expect(trans.guard).toBe('x > 0');
            expect(trans.action).toBe('onStart()');
            expect(typeof trans.id).toBe('string');
        });

        test('creates transition with empty optional fields', function() {
            var trans = model.createTransitionConfig('n1', 'n2', '', '', '');
            expect(trans.event).toBe('');
            expect(trans.guard).toBe('');
            expect(trans.action).toBe('');
        });

        test('handles null/undefined optional fields gracefully', function() {
            var trans = model.createTransitionConfig('n1', 'n2', null, undefined, null);
            expect(trans.event).toBe('');
            expect(trans.guard).toBe('');
            expect(trans.action).toBe('');
        });

        test('returns null for empty from', function() {
            expect(model.createTransitionConfig('', 'n2', 'EV')).toBeNull();
        });

        test('returns null for empty to', function() {
            expect(model.createTransitionConfig('n1', '', 'EV')).toBeNull();
        });

        test('returns null for non-string from', function() {
            expect(model.createTransitionConfig(42, 'n2', 'EV')).toBeNull();
        });
    });

    describe('addState', function() {
        test('adds a state to the FSM', function() {
            var state = model.addState('Idle', 'normal');
            expect(state).not.toBeNull();
            expect(model.getFsm().states).toHaveLength(1);
            expect(model.getFsm().states[0].name).toBe('Idle');
        });

        test('prevents duplicate state names', function() {
            model.addState('Idle', 'normal');
            var dup = model.addState('Idle', 'final');
            expect(dup).toBeNull();
            expect(model.getFsm().states).toHaveLength(1);
        });

        test('sets initial state ID on FSM when adding initial type', function() {
            var state = model.addState('Start', 'initial');
            expect(model.getFsm().initial).toBe(state.id);
        });

        test('does not overwrite initial if already set', function() {
            var s1 = model.addState('Start', 'initial');
            model.addState('S2', 'initial');
            expect(model.getFsm().initial).toBe(s1.id);
        });

        test('returns null for empty name', function() {
            expect(model.addState('', 'normal')).toBeNull();
        });

        test('trims state names', function() {
            var state = model.addState('  Running  ', 'normal');
            expect(state.name).toBe('Running');
        });
    });

    describe('removeState', function() {
        test('removes a state by ID', function() {
            var state = model.addState('Idle', 'normal');
            expect(model.removeState(state.id)).toBe(true);
            expect(model.getFsm().states).toHaveLength(0);
        });

        test('removes associated transitions', function() {
            var s1 = model.addState('Idle', 'initial');
            var s2 = model.addState('Running', 'normal');
            model.addTransition(s1.id, s2.id, 'START');
            expect(model.getFsm().transitions).toHaveLength(1);
            model.removeState(s2.id);
            expect(model.getFsm().transitions).toHaveLength(0);
        });

        test('clears initial if the initial state is removed', function() {
            var state = model.addState('Start', 'initial');
            model.removeState(state.id);
            expect(model.getFsm().initial).toBe('');
        });

        test('returns false for unknown ID', function() {
            expect(model.removeState('nonexistent')).toBe(false);
        });
    });

    describe('addTransition', function() {
        test('adds a transition between existing states', function() {
            var s1 = model.addState('Idle', 'initial');
            var s2 = model.addState('Running', 'normal');
            var trans = model.addTransition(s1.id, s2.id, 'START', 'isReady()', 'onStart()');
            expect(trans).not.toBeNull();
            expect(model.getFsm().transitions).toHaveLength(1);
            expect(trans.event).toBe('START');
        });

        test('returns null if from state does not exist', function() {
            var s2 = model.addState('Running', 'normal');
            expect(model.addTransition('bogus', s2.id, 'EV')).toBeNull();
        });

        test('returns null if to state does not exist', function() {
            var s1 = model.addState('Idle', 'normal');
            expect(model.addTransition(s1.id, 'bogus', 'EV')).toBeNull();
        });
    });

    describe('removeTransition', function() {
        test('removes a transition by ID', function() {
            var s1 = model.addState('Idle', 'initial');
            var s2 = model.addState('Running', 'normal');
            var trans = model.addTransition(s1.id, s2.id, 'START');
            expect(model.removeTransition(trans.id)).toBe(true);
            expect(model.getFsm().transitions).toHaveLength(0);
        });

        test('returns false for unknown transition ID', function() {
            expect(model.removeTransition('nonexistent')).toBe(false);
        });
    });

    describe('resetFsm', function() {
        test('resets FSM to empty state', function() {
            model.addState('Idle', 'initial');
            model.addState('Running', 'normal');
            model.resetFsm();
            expect(model.getFsm().states).toHaveLength(0);
            expect(model.getFsm().transitions).toHaveLength(0);
            expect(model.getFsm().initial).toBe('');
            expect(model.getFsm().name).toBe('MyFSM');
        });
    });

    describe('undo/redo', function() {
        test('undo restores previous state', function() {
            model.addState('Idle', 'initial');
            model.snap();
            model.addState('Running', 'normal');
            expect(model.getFsm().states).toHaveLength(2);
            model.undo();
            expect(model.getFsm().states).toHaveLength(1);
        });

        test('redo restores undone state', function() {
            model.addState('Idle', 'initial');
            model.snap();
            model.addState('Running', 'normal');
            model.undo();
            expect(model.getFsm().states).toHaveLength(1);
            model.redo();
            expect(model.getFsm().states).toHaveLength(2);
        });

        test('undo returns false when no history', function() {
            expect(model.undo()).toBe(false);
        });

        test('redo returns false when no redo history', function() {
            expect(model.redo()).toBe(false);
        });

        test('snap clears redo history', function() {
            model.addState('Idle', 'initial');
            model.snap();
            model.addState('Running', 'normal');
            model.undo();
            expect(model.getRedoLength()).toBe(1);
            model.snap();
            expect(model.getRedoLength()).toBe(0);
        });

        test('history is capped at 80 entries', function() {
            for (var i = 0; i < 90; i++) {
                model.snap();
            }
            expect(model.getHistoryLength()).toBeLessThanOrEqual(80);
        });
    });

    describe('nid', function() {
        test('generates unique IDs', function() {
            model.resetFsm();
            var id1 = model.nid();
            var id2 = model.nid();
            expect(id1).not.toBe(id2);
            expect(id1).toMatch(/^n\d+$/);
            expect(id2).toMatch(/^n\d+$/);
        });
    });
});
