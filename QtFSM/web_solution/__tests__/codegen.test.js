// codegen.test.js — Jest tests for code generation module
'use strict';

var codegen = require('../js/codegen.js');

describe('Codegen Module', function() {

    var testFsm;

    beforeEach(function() {
        testFsm = {
            name: 'TestFSM',
            initial: 's1',
            states: [
                {
                    id: 's1', name: 'Idle', type: 'initial', x: 100, y: 100,
                    onEntry: 'initLed();', onExit: 'cleanupLed();',
                    entryGuard: '', exitGuard: '',
                    internalEvents: [], comment: '', history: 'none'
                },
                {
                    id: 's2', name: 'Running', type: 'normal', x: 200, y: 200,
                    onEntry: 'startTimer();', onExit: 'stopTimer();',
                    entryGuard: '', exitGuard: '',
                    internalEvents: [{ event: 'TICK', guard: '', action: 'update()' }],
                    comment: 'Main state', history: 'none'
                },
                {
                    id: 's3', name: 'Stopped', type: 'final', x: 300, y: 300,
                    onEntry: 'onStop();', onExit: '',
                    entryGuard: '', exitGuard: '',
                    internalEvents: [], comment: '', history: 'none'
                }
            ],
            transitions: [
                { id: 't1', from: 's1', to: 's2', event: 'START', guard: '', action: 'onStart()' },
                { id: 't2', from: 's2', to: 's3', event: 'STOP', guard: 'ready', action: 'onStop()' }
            ]
        };
    });

    describe('buildHeaderSection', function() {
        test('generates header with #pragma once', function() {
            var header = codegen.buildHeaderSection(testFsm, '');
            expect(header).toContain('#pragma once');
        });

        test('generates state enum', function() {
            var header = codegen.buildHeaderSection(testFsm, '');
            expect(header).toContain('IDLE');
            expect(header).toContain('RUNNING');
            expect(header).toContain('STOPPED');
        });

        test('generates event enum', function() {
            var header = codegen.buildHeaderSection(testFsm, '');
            expect(header).toContain('START');
            expect(header).toContain('STOP');
        });

        test('generates class name', function() {
            var header = codegen.buildHeaderSection(testFsm, '');
            expect(header).toContain('class TestFSM');
        });

        test('generates initial state in constructor', function() {
            var header = codegen.buildHeaderSection(testFsm, '');
            expect(header).toContain('state_(State::IDLE)');
        });

        test('generates isFinal check for final states', function() {
            var header = codegen.buildHeaderSection(testFsm, '');
            expect(header).toContain('state_ == State::STOPPED');
        });

        test('wraps in namespace when provided', function() {
            var header = codegen.buildHeaderSection(testFsm, 'myns');
            expect(header).toContain('namespace myns {');
            expect(header).toContain('} // namespace myns');
        });

        test('returns empty for null FSM', function() {
            expect(codegen.buildHeaderSection(null, '')).toBe('');
        });
    });

    describe('buildSourceSection', function() {
        test('generates #include for the header', function() {
            var source = codegen.buildSourceSection(testFsm, '');
            expect(source).toContain('#include "TestFSM.h"');
        });

        test('generates process function with transitions', function() {
            var source = codegen.buildSourceSection(testFsm, '');
            expect(source).toContain('TestFSM::process(Event event)');
            expect(source).toContain('case State::IDLE:');
            expect(source).toContain('Event::START');
            expect(source).toContain('state_ = State::RUNNING');
        });

        test('generates guard comments', function() {
            var source = codegen.buildSourceSection(testFsm, '');
            expect(source).toContain('// guard: ready');
        });

        test('generates action calls', function() {
            var source = codegen.buildSourceSection(testFsm, '');
            expect(source).toContain('onStart();');
        });

        test('generates onEntry handlers', function() {
            var source = codegen.buildSourceSection(testFsm, '');
            expect(source).toContain('onEntry()');
            expect(source).toContain('initLed();');
            expect(source).toContain('startTimer();');
        });

        test('generates onExit handlers', function() {
            var source = codegen.buildSourceSection(testFsm, '');
            expect(source).toContain('onExit()');
            expect(source).toContain('cleanupLed();');
            expect(source).toContain('stopTimer();');
        });

        test('generates stateName function', function() {
            var source = codegen.buildSourceSection(testFsm, '');
            expect(source).toContain('stateName()');
            expect(source).toContain('return "Idle"');
            expect(source).toContain('return "Running"');
        });

        test('returns empty for null FSM', function() {
            expect(codegen.buildSourceSection(null, '')).toBe('');
        });
    });

    describe('buildCpp', function() {
        test('includes header begin marker', function() {
            var cpp = codegen.buildCpp(testFsm, '');
            expect(cpp).toContain(codegen.MARKER_HEADER_BEGIN);
        });

        test('includes header end marker', function() {
            var cpp = codegen.buildCpp(testFsm, '');
            expect(cpp).toContain(codegen.MARKER_HEADER_END);
        });

        test('includes source begin marker', function() {
            var cpp = codegen.buildCpp(testFsm, '');
            expect(cpp).toContain(codegen.MARKER_SOURCE_BEGIN);
        });

        test('includes source end marker', function() {
            var cpp = codegen.buildCpp(testFsm, '');
            expect(cpp).toContain(codegen.MARKER_SOURCE_END);
        });

        test('header section appears before source section', function() {
            var cpp = codegen.buildCpp(testFsm, '');
            var hBegin = cpp.indexOf(codegen.MARKER_HEADER_BEGIN);
            var hEnd = cpp.indexOf(codegen.MARKER_HEADER_END);
            var sBegin = cpp.indexOf(codegen.MARKER_SOURCE_BEGIN);
            var sEnd = cpp.indexOf(codegen.MARKER_SOURCE_END);
            expect(hBegin).toBeLessThan(hEnd);
            expect(hEnd).toBeLessThan(sBegin);
            expect(sBegin).toBeLessThan(sEnd);
        });

        test('includes FSM metadata comment', function() {
            var cpp = codegen.buildCpp(testFsm, '');
            expect(cpp).toContain('// FSM:    TestFSM');
            expect(cpp).toContain('// States: 3');
        });

        test('handles empty states FSM', function() {
            var emptyFsm = { name: 'Empty', initial: '', states: [], transitions: [] };
            var cpp = codegen.buildCpp(emptyFsm, '');
            expect(cpp).toContain(codegen.MARKER_HEADER_BEGIN);
            expect(cpp).toContain(codegen.MARKER_SOURCE_BEGIN);
        });
    });

    describe('buildJson', function() {
        test('returns valid JSON', function() {
            var json = codegen.buildJson(testFsm);
            expect(function() { JSON.parse(json); }).not.toThrow();
        });

        test('contains FSM name', function() {
            var obj = JSON.parse(codegen.buildJson(testFsm));
            expect(obj.fsm.name).toBe('TestFSM');
        });

        test('contains states with names', function() {
            var obj = JSON.parse(codegen.buildJson(testFsm));
            expect(obj.fsm.states).toHaveLength(3);
            var names = obj.fsm.states.map(function(s) { return s.name; });
            expect(names).toContain('Idle');
            expect(names).toContain('Running');
            expect(names).toContain('Stopped');
        });

        test('contains transitions', function() {
            var obj = JSON.parse(codegen.buildJson(testFsm));
            expect(obj.fsm.transitions).toHaveLength(2);
        });

        test('contains initial state name', function() {
            var obj = JSON.parse(codegen.buildJson(testFsm));
            expect(obj.fsm.initial).toBe('Idle');
        });

        test('contains meta info', function() {
            var obj = JSON.parse(codegen.buildJson(testFsm));
            expect(obj.meta.generator).toBe('FSM Creator');
            expect(obj.meta.version).toBe('2.0');
        });

        test('returns empty JSON for null FSM', function() {
            expect(codegen.buildJson(null)).toBe('{}');
        });
    });

    describe('round-trip: build C++ -> parseable', function() {
        test('generated C++ contains parseable markers', function() {
            var parser = require('../js/parser.js');
            var cpp = codegen.buildCpp(testFsm, '');

            /* Verify the markers are present and content extractable */
            var header = parser.extractSection(
                cpp, parser.MARKER_HEADER_BEGIN, parser.MARKER_HEADER_END
            );
            var source = parser.extractSection(
                cpp, parser.MARKER_SOURCE_BEGIN, parser.MARKER_SOURCE_END
            );
            expect(header).not.toBeNull();
            expect(source).not.toBeNull();
            expect(header).toContain('class TestFSM');
            expect(source).toContain('#include "TestFSM.h"');
        });
    });
});
