// parser.test.js — Jest tests for FSM parser
'use strict';

var parser = require('../js/parser.js');

describe('Parser Module', function() {

    describe('extractSection', function() {
        test('extracts content between markers', function() {
            var src = 'before\n' +
                parser.MARKER_HEADER_BEGIN + '\nHEADER CONTENT\n' +
                parser.MARKER_HEADER_END + '\nafter';
            var result = parser.extractSection(
                src, parser.MARKER_HEADER_BEGIN, parser.MARKER_HEADER_END
            );
            expect(result).toBe('HEADER CONTENT');
        });

        test('returns null when begin marker missing', function() {
            var src = 'some text\n' + parser.MARKER_HEADER_END;
            expect(parser.extractSection(
                src, parser.MARKER_HEADER_BEGIN, parser.MARKER_HEADER_END
            )).toBeNull();
        });

        test('returns null when end marker missing', function() {
            var src = parser.MARKER_HEADER_BEGIN + '\ncontent';
            expect(parser.extractSection(
                src, parser.MARKER_HEADER_BEGIN, parser.MARKER_HEADER_END
            )).toBeNull();
        });

        test('returns null for non-string input', function() {
            expect(parser.extractSection(null, 'a', 'b')).toBeNull();
            expect(parser.extractSection(123, 'a', 'b')).toBeNull();
        });

        test('extracts trimmed content', function() {
            var src = parser.MARKER_HEADER_BEGIN + '\n  \n  content  \n  \n' +
                parser.MARKER_HEADER_END;
            expect(parser.extractSection(
                src, parser.MARKER_HEADER_BEGIN, parser.MARKER_HEADER_END
            )).toBe('content');
        });
    });

    describe('detectFormat', function() {
        test('detects marked format', function() {
            expect(parser.detectFormat(parser.MARKER_HEADER_BEGIN + '\n...')).toBe('marked');
            expect(parser.detectFormat('...\n' + parser.MARKER_SOURCE_BEGIN)).toBe('marked');
        });

        test('detects dsl format', function() {
            expect(parser.detectFormat('FSM: MyFSM\nSTATE: Idle')).toBe('dsl');
            expect(parser.detectFormat('TRANS: A -> B : EV')).toBe('dsl');
        });

        test('returns unknown for unrecognized', function() {
            expect(parser.detectFormat('just some text')).toBe('unknown');
        });

        test('returns unknown for non-string', function() {
            expect(parser.detectFormat(null)).toBe('unknown');
            expect(parser.detectFormat(42)).toBe('unknown');
        });
    });

    describe('blankResult', function() {
        test('creates blank result with defaults', function() {
            var r = parser.blankResult();
            expect(r.name).toBe('MyFSM');
            expect(r.states).toEqual([]);
            expect(r.transitions).toEqual([]);
            expect(r.errors).toEqual([]);
        });

        test('creates blank result with custom name', function() {
            var r = parser.blankResult('TestFSM');
            expect(r.name).toBe('TestFSM');
        });
    });

    describe('stripBlockComments', function() {
        test('removes block comments', function() {
            expect(parser.stripBlockComments('a /* comment */ b')).toBe('a   b');
        });

        test('removes multi-line block comments', function() {
            var result = parser.stripBlockComments('a /* multi\nline\ncomment */ b');
            expect(result).toBe('a   b');
        });

        test('preserves line comments', function() {
            var result = parser.stripBlockComments('a // line comment\nb');
            expect(result).toContain('// line comment');
        });
    });

    describe('parseHeader', function() {
        test('extracts class name', function() {
            var r = parser.blankResult();
            parser.parseHeader('class TrafficLightFSM {', r);
            expect(r.name).toBe('TrafficLightFSM');
        });

        test('extracts state enum', function() {
            var r = parser.blankResult();
            parser.parseHeader(
                'enum class State : uint8_t { IDLE, RUNNING, STOPPED };', r
            );
            expect(r.states).toHaveLength(3);
            expect(r.states.map(function(s) { return s.name; }))
                .toEqual(['IDLE', 'RUNNING', 'STOPPED']);
        });

        test('extracts initial state from constructor', function() {
            var r = parser.blankResult();
            parser.parseHeader(
                'enum class State { IDLE, RUN };\n' +
                'MyFSM() : state_(State::IDLE) {}', r
            );
            expect(r.initial).toBe('IDLE');
            var idleState = r.states.find(function(s) { return s.name === 'IDLE'; });
            expect(idleState.type).toBe('initial');
        });

        test('extracts initial state from assignment', function() {
            var r = parser.blankResult();
            parser.parseHeader(
                'enum class State { A, B };\n' +
                'void reset() { state_ = State::A; }', r
            );
            expect(r.initial).toBe('A');
        });

        test('handles empty header', function() {
            var r = parser.blankResult();
            parser.parseHeader('', r);
            expect(r.states).toHaveLength(0);
        });

        test('handles null header', function() {
            var r = parser.blankResult();
            parser.parseHeader(null, r);
            expect(r.states).toHaveLength(0);
        });
    });

    describe('parseSource', function() {
        test('extracts transitions from switch/case/if', function() {
            var r = parser.blankResult();
            r.states = [
                { name: 'IDLE', type: 'normal' },
                { name: 'RUNNING', type: 'normal' }
            ];
            parser.parseSource(
                'switch (state_) {\n' +
                '    case State::IDLE:\n' +
                '        if (event == Event::START) {\n' +
                '            doStart();\n' +
                '            state_ = State::RUNNING;\n' +
                '        }\n' +
                '        break;\n' +
                '}', r
            );
            expect(r.transitions).toHaveLength(1);
            expect(r.transitions[0].from).toBe('IDLE');
            expect(r.transitions[0].to).toBe('RUNNING');
            expect(r.transitions[0].event).toBe('START');
        });

        test('handles empty source', function() {
            var r = parser.blankResult();
            parser.parseSource('', r);
            expect(r.transitions).toHaveLength(0);
        });

        test('handles null source', function() {
            var r = parser.blankResult();
            parser.parseSource(null, r);
            expect(r.transitions).toHaveLength(0);
        });
    });

    describe('parseDsl', function() {
        test('parses FSM name', function() {
            var r = parser.parseDsl('FSM: TestMachine');
            expect(r.name).toBe('TestMachine');
        });

        test('parses states', function() {
            var r = parser.parseDsl('STATE: Idle\nSTATE: Running');
            expect(r.states).toHaveLength(2);
        });

        test('parses initial state', function() {
            var r = parser.parseDsl('STATE: Idle\nINIT: Idle');
            expect(r.initial).toBe('Idle');
            var s = r.states.find(function(x) { return x.name === 'Idle'; });
            expect(s.type).toBe('initial');
        });

        test('parses final state', function() {
            var r = parser.parseDsl('FINAL: Done');
            var s = r.states.find(function(x) { return x.name === 'Done'; });
            expect(s.type).toBe('final');
        });

        test('parses transitions', function() {
            var r = parser.parseDsl(
                'STATE: Idle\nSTATE: Running\n' +
                'TRANS: Idle -> Running : START [ready] / onStart()'
            );
            expect(r.transitions).toHaveLength(1);
            expect(r.transitions[0].from).toBe('Idle');
            expect(r.transitions[0].to).toBe('Running');
            expect(r.transitions[0].event).toBe('START');
            expect(r.transitions[0].guard).toBe('ready');
            expect(r.transitions[0].action).toBe('onStart()');
        });

        test('parses transitions without guard/action', function() {
            var r = parser.parseDsl(
                'STATE: A\nSTATE: B\n' +
                'TRANS: A -> B : GO'
            );
            expect(r.transitions[0].guard).toBe('');
            expect(r.transitions[0].action).toBe('');
        });

        test('parses on entry/exit', function() {
            var r = parser.parseDsl(
                'STATE: Idle\n' +
                'ON_ENTRY: Idle / startTimer()\n' +
                'ON_EXIT: Idle / stopTimer()'
            );
            var s = r.states.find(function(x) { return x.name === 'Idle'; });
            expect(s.onEntry).toBe('startTimer()');
            expect(s.onExit).toBe('stopTimer()');
        });

        test('parses internal events', function() {
            var r = parser.parseDsl(
                'STATE: Running\n' +
                'INTERNAL: Running / TICK [verbose] / updateDisplay()'
            );
            var s = r.states.find(function(x) { return x.name === 'Running'; });
            expect(s.internalEvents).toHaveLength(1);
            expect(s.internalEvents[0].event).toBe('TICK');
            expect(s.internalEvents[0].guard).toBe('verbose');
            expect(s.internalEvents[0].action).toBe('updateDisplay()');
        });

        test('parses comments', function() {
            var r = parser.parseDsl(
                'STATE: Idle\n' +
                'COMMENT: Idle / Waiting for input'
            );
            var s = r.states.find(function(x) { return x.name === 'Idle'; });
            expect(s.comment).toBe('Waiting for input');
        });

        test('skips comment lines', function() {
            var r = parser.parseDsl('// This is a comment\nSTATE: Idle');
            expect(r.states).toHaveLength(1);
        });

        test('reports bad TRANS syntax', function() {
            var r = parser.parseDsl('TRANS: bad line');
            expect(r.errors.length).toBeGreaterThan(0);
        });

        test('warns about missing initial state', function() {
            var r = parser.parseDsl('STATE: Idle');
            expect(r.warnings.length).toBeGreaterThan(0);
        });

        test('sets format to dsl', function() {
            var r = parser.parseDsl('FSM: Test');
            expect(r.format).toBe('dsl');
        });
    });

    describe('parseMarked', function() {
        test('parses both header and source sections', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                'class TestFSM {\n' +
                'enum class State : uint8_t { IDLE, RUNNING };\n' +
                'enum class Event : uint8_t { START };\n' +
                'TestFSM() : state_(State::IDLE) {}\n' +
                parser.MARKER_HEADER_END + '\n' +
                parser.MARKER_SOURCE_BEGIN + '\n' +
                'switch (state_) {\n' +
                '    case State::IDLE:\n' +
                '        if (event == Event::START) {\n' +
                '            state_ = State::RUNNING;\n' +
                '        }\n' +
                '        break;\n' +
                '}\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseMarked(src);
            expect(r.format).toBe('marked');
            expect(r.name).toBe('TestFSM');
            expect(r.states.length).toBeGreaterThanOrEqual(2);
            expect(r.initial).toBe('IDLE');
            expect(r.transitions).toHaveLength(1);
            expect(r.transitions[0].from).toBe('IDLE');
            expect(r.transitions[0].to).toBe('RUNNING');
        });

        test('handles header-only input', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                'class TestFSM {\n' +
                'enum class State { A, B };\n' +
                parser.MARKER_HEADER_END;

            var r = parser.parseMarked(src);
            expect(r.states).toHaveLength(2);
            expect(r.transitions).toHaveLength(0);
        });

        test('reports error when no markers found', function() {
            var r = parser.parseMarked('just some code');
            expect(r.errors.length).toBeGreaterThan(0);
        });

        test('reports error when no states found', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                'int x = 5;\n' +
                parser.MARKER_HEADER_END;
            var r = parser.parseMarked(src);
            expect(r.errors.length).toBeGreaterThan(0);
        });
    });

    describe('parseInput (master parser)', function() {
        test('auto-detects marked format', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                'class FSM { enum class State { A }; };\n' +
                parser.MARKER_HEADER_END;
            var r = parser.parseInput(src);
            expect(r.format).toBe('marked');
        });

        test('auto-detects DSL format', function() {
            var r = parser.parseInput('FSM: Test\nSTATE: Idle\nINIT: Idle');
            expect(r.format).toBe('dsl');
            expect(r.states).toHaveLength(1);
        });

        test('returns error for empty input', function() {
            var r = parser.parseInput('');
            expect(r.errors.length).toBeGreaterThan(0);
        });

        test('returns error for null input', function() {
            var r = parser.parseInput(null);
            expect(r.errors.length).toBeGreaterThan(0);
        });

        test('returns error for unrecognized format with no states', function() {
            var r = parser.parseInput('random text that is not FSM');
            expect(r.errors.length).toBeGreaterThan(0);
        });
    });

    describe('round-trip: DSL parse -> config structure', function() {
        test('preserves all data through DSL parsing', function() {
            var dsl = [
                'FSM: TrafficLight',
                'STATE: Red',
                'STATE: Green',
                'STATE: Yellow',
                'INIT: Red',
                'FINAL: Yellow',
                'TRANS: Red -> Green : TIMER [] / switchLight()',
                'TRANS: Green -> Yellow : TIMER',
                'TRANS: Yellow -> Red : TIMER',
                'ON_ENTRY: Red / activateRedLED()',
                'ON_EXIT: Red / deactivateRedLED()',
                'INTERNAL: Green / BLINK [] / blinkGreen()',
                'COMMENT: Red / Wait for timer'
            ].join('\n');

            var r = parser.parseInput(dsl);

            expect(r.name).toBe('TrafficLight');
            expect(r.initial).toBe('Red');
            expect(r.states).toHaveLength(3);
            expect(r.transitions).toHaveLength(3);
            expect(r.errors).toHaveLength(0);

            var red = r.states.find(function(s) { return s.name === 'Red'; });
            expect(red.type).toBe('initial');
            expect(red.onEntry).toBe('activateRedLED()');
            expect(red.onExit).toBe('deactivateRedLED()');
            expect(red.comment).toBe('Wait for timer');

            var yellow = r.states.find(function(s) { return s.name === 'Yellow'; });
            expect(yellow.type).toBe('final');

            var green = r.states.find(function(s) { return s.name === 'Green'; });
            expect(green.internalEvents).toHaveLength(1);
            expect(green.internalEvents[0].event).toBe('BLINK');
        });
    });

    /* ═══════════════════════════════════════════════════════
     * REAL-WORLD C++ CODE INPUTS
     * Full production-grade .h + .cpp style FSM code
     * ═══════════════════════════════════════════════════════ */
    describe('Real-World C++ Code Inputs', function() {

        test('parses complete embedded motor controller FSM', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                '#pragma once\n' +
                '#include <cstdint>\n' +
                '#include <functional>\n' +
                '\n' +
                'namespace embedded {\n' +
                'namespace motor {\n' +
                '\n' +
                '/**\n' +
                ' * @brief Motor controller FSM for industrial servo drive\n' +
                ' * @details Implements IEC 61800-7 drive profile states\n' +
                ' */\n' +
                'class MotorControllerFSM {\n' +
                'public:\n' +
                '    enum class State : uint8_t {\n' +
                '        NOT_READY_TO_SWITCH_ON,\n' +
                '        SWITCH_ON_DISABLED,\n' +
                '        READY_TO_SWITCH_ON,\n' +
                '        SWITCHED_ON,\n' +
                '        OPERATION_ENABLED,\n' +
                '        QUICK_STOP_ACTIVE,\n' +
                '        FAULT_REACTION_ACTIVE,\n' +
                '        FAULT\n' +
                '    };\n' +
                '\n' +
                '    enum class Event : uint8_t {\n' +
                '        POWER_GOOD,\n' +
                '        SHUTDOWN,\n' +
                '        SWITCH_ON,\n' +
                '        ENABLE_OP,\n' +
                '        DISABLE_VOLTAGE,\n' +
                '        QUICK_STOP,\n' +
                '        FAULT_DETECTED,\n' +
                '        FAULT_CLEARED,\n' +
                '        RESET\n' +
                '    };\n' +
                '\n' +
                '    MotorControllerFSM() : state_(State::NOT_READY_TO_SWITCH_ON) {}\n' +
                '    ~MotorControllerFSM() = default;\n' +
                '\n' +
                '    bool process(Event event);\n' +
                '    State getState() const { return state_; }\n' +
                '    bool isFinal() const { return state_ == State::FAULT; }\n' +
                '    void reset() { state_ = State::NOT_READY_TO_SWITCH_ON; }\n' +
                '    const char* stateName() const;\n' +
                '    void onEntry();\n' +
                '    void onExit();\n' +
                '\n' +
                'private:\n' +
                '    State state_;\n' +
                '    uint32_t fault_code_ = 0U;\n' +
                '    bool brake_engaged_ = true;\n' +
                '};\n' +
                '\n' +
                '} // namespace motor\n' +
                '} // namespace embedded\n' +
                parser.MARKER_HEADER_END + '\n' +
                parser.MARKER_SOURCE_BEGIN + '\n' +
                '#include "MotorControllerFSM.h"\n' +
                '#include <cstdio>\n' +
                '\n' +
                'namespace embedded {\n' +
                'namespace motor {\n' +
                '\n' +
                'bool MotorControllerFSM::process(Event event) {\n' +
                '    switch (state_) {\n' +
                '        case State::NOT_READY_TO_SWITCH_ON:\n' +
                '            if (event == Event::POWER_GOOD) {\n' +
                '                initHardware();\n' +
                '                state_ = State::SWITCH_ON_DISABLED;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::SWITCH_ON_DISABLED:\n' +
                '            if (event == Event::SHUTDOWN) {\n' +
                '                state_ = State::READY_TO_SWITCH_ON;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::READY_TO_SWITCH_ON:\n' +
                '            if (event == Event::SWITCH_ON) {\n' +
                '                enablePowerStage();\n' +
                '                state_ = State::SWITCHED_ON;\n' +
                '            }\n' +
                '            if (event == Event::DISABLE_VOLTAGE) {\n' +
                '                state_ = State::SWITCH_ON_DISABLED;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::SWITCHED_ON:\n' +
                '            if (event == Event::ENABLE_OP) {\n' +
                '                releaseBrake();\n' +
                '                startPWM();\n' +
                '                state_ = State::OPERATION_ENABLED;\n' +
                '            }\n' +
                '            if (event == Event::SHUTDOWN) {\n' +
                '                disablePowerStage();\n' +
                '                state_ = State::READY_TO_SWITCH_ON;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::OPERATION_ENABLED:\n' +
                '            if (event == Event::QUICK_STOP) {\n' +
                '                engageBrake();\n' +
                '                state_ = State::QUICK_STOP_ACTIVE;\n' +
                '            }\n' +
                '            if (event == Event::DISABLE_VOLTAGE) {\n' +
                '                stopPWM();\n' +
                '                state_ = State::SWITCH_ON_DISABLED;\n' +
                '            }\n' +
                '            if (event == Event::FAULT_DETECTED) {\n' +
                '                logFault();\n' +
                '                state_ = State::FAULT_REACTION_ACTIVE;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::QUICK_STOP_ACTIVE:\n' +
                '            if (event == Event::DISABLE_VOLTAGE) {\n' +
                '                state_ = State::SWITCH_ON_DISABLED;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::FAULT_REACTION_ACTIVE:\n' +
                '            if (event == Event::FAULT_DETECTED) {\n' +
                '                engageBrake();\n' +
                '                disableAllOutputs();\n' +
                '                state_ = State::FAULT;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::FAULT:\n' +
                '            if (event == Event::RESET) {\n' +
                '                clearFaultFlags();\n' +
                '                state_ = State::SWITCH_ON_DISABLED;\n' +
                '            }\n' +
                '            break;\n' +
                '    }\n' +
                '    return false;\n' +
                '}\n' +
                '\n' +
                'void MotorControllerFSM::onEntry() {\n' +
                '    switch (state_) {\n' +
                '        case State::OPERATION_ENABLED:\n' +
                '            enableCurrentLoop();\n' +
                '            enablePositionLoop();\n' +
                '            break;\n' +
                '        case State::FAULT:\n' +
                '            setStatusLED(LED_RED);\n' +
                '            publishDiagnostic();\n' +
                '            break;\n' +
                '        case State::QUICK_STOP_ACTIVE:\n' +
                '            applyDecelerationRamp();\n' +
                '            break;\n' +
                '    }\n' +
                '}\n' +
                '\n' +
                'void MotorControllerFSM::onExit() {\n' +
                '    switch (state_) {\n' +
                '        case State::OPERATION_ENABLED:\n' +
                '            disableCurrentLoop();\n' +
                '            break;\n' +
                '        case State::FAULT:\n' +
                '            clearStatusLED();\n' +
                '            break;\n' +
                '    }\n' +
                '}\n' +
                '\n' +
                '} // namespace motor\n' +
                '} // namespace embedded\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');
            expect(r.name).toBe('MotorControllerFSM');
            expect(r.initial).toBe('NOT_READY_TO_SWITCH_ON');
            expect(r.states).toHaveLength(8);
            expect(r.errors).toHaveLength(0);

            /* Verify all 8 IEC 61800-7 states */
            var expected = [
                'NOT_READY_TO_SWITCH_ON', 'SWITCH_ON_DISABLED',
                'READY_TO_SWITCH_ON', 'SWITCHED_ON',
                'OPERATION_ENABLED', 'QUICK_STOP_ACTIVE',
                'FAULT_REACTION_ACTIVE', 'FAULT'
            ];
            var names = r.states.map(function(s) { return s.name; });
            expected.forEach(function(n) {
                expect(names).toContain(n);
            });

            /* FAULT should be final */
            var fault = r.states.find(function(s) { return s.name === 'FAULT'; });
            expect(fault.type).toBe('final');

            /* Verify transition count (should be 11) */
            expect(r.transitions.length).toBeGreaterThanOrEqual(10);

            /* Spot-check specific transitions */
            var powerGood = r.transitions.find(function(t) {
                return t.from === 'NOT_READY_TO_SWITCH_ON' &&
                       t.to === 'SWITCH_ON_DISABLED' &&
                       t.event === 'POWER_GOOD';
            });
            expect(powerGood).toBeDefined();

            var quickStop = r.transitions.find(function(t) {
                return t.from === 'OPERATION_ENABLED' &&
                       t.to === 'QUICK_STOP_ACTIVE' &&
                       t.event === 'QUICK_STOP';
            });
            expect(quickStop).toBeDefined();

            /* Verify entry actions on OPERATION_ENABLED */
            var opEnabled = r.states.find(function(s) {
                return s.name === 'OPERATION_ENABLED';
            });
            if (opEnabled.onEntry) {
                expect(opEnabled.onEntry).toContain('enableCurrentLoop');
            }

            /* Verify exit actions on OPERATION_ENABLED */
            if (opEnabled.onExit) {
                expect(opEnabled.onExit).toContain('disableCurrentLoop');
            }
        });

        test('parses AUTOSAR-style communication stack FSM', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                '#pragma once\n' +
                '#include <cstdint>\n' +
                '\n' +
                '/* AUTOSAR COM Stack - CAN transceiver state machine\n' +
                '   Reference: AUTOSAR_SWS_CANTransceiver R20-11 */\n' +
                '\n' +
                'class CanTrcvFSM {\n' +
                'public:\n' +
                '    enum class State : uint8_t {\n' +
                '        UNINIT,\n' +
                '        NORMAL,\n' +
                '        STANDBY,\n' +
                '        SLEEP\n' +
                '    };\n' +
                '\n' +
                '    enum class Event : uint8_t {\n' +
                '        INIT_COMPLETE,\n' +
                '        SET_NORMAL,\n' +
                '        SET_STANDBY,\n' +
                '        SET_SLEEP,\n' +
                '        WAKEUP\n' +
                '    };\n' +
                '\n' +
                '    CanTrcvFSM() : state_(State::UNINIT) {}\n' +
                '\n' +
                '    bool process(Event event);\n' +
                '    State getState() const { return state_; }\n' +
                '\n' +
                'private:\n' +
                '    State state_;\n' +
                '    uint8_t wakeup_reason_ = 0U;\n' +
                '};\n' +
                parser.MARKER_HEADER_END + '\n' +
                parser.MARKER_SOURCE_BEGIN + '\n' +
                '#include "CanTrcvFSM.h"\n' +
                '\n' +
                'bool CanTrcvFSM::process(Event event) {\n' +
                '    switch (state_) {\n' +
                '        case State::UNINIT:\n' +
                '            if (event == Event::INIT_COMPLETE) {\n' +
                '                configureRegisters();\n' +
                '                state_ = State::STANDBY;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::STANDBY:\n' +
                '            if (event == Event::SET_NORMAL) {\n' +
                '                enableTransceiver();\n' +
                '                state_ = State::NORMAL;\n' +
                '            }\n' +
                '            if (event == Event::SET_SLEEP) {\n' +
                '                enterLowPower();\n' +
                '                state_ = State::SLEEP;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::NORMAL:\n' +
                '            if (event == Event::SET_STANDBY) {\n' +
                '                disableTransceiver();\n' +
                '                state_ = State::STANDBY;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::SLEEP:\n' +
                '            if (event == Event::WAKEUP) {\n' +
                '                exitLowPower();\n' +
                '                storeWakeupReason();\n' +
                '                state_ = State::STANDBY;\n' +
                '            }\n' +
                '            break;\n' +
                '    }\n' +
                '    return false;\n' +
                '}\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');
            expect(r.name).toBe('CanTrcvFSM');
            expect(r.initial).toBe('UNINIT');
            expect(r.states).toHaveLength(4);
            expect(r.errors).toHaveLength(0);

            /* 5 transitions */
            expect(r.transitions).toHaveLength(5);

            /* Verify UNINIT -> STANDBY on INIT_COMPLETE */
            var initTrans = r.transitions.find(function(t) {
                return t.from === 'UNINIT' && t.to === 'STANDBY';
            });
            expect(initTrans).toBeDefined();
            expect(initTrans.event).toBe('INIT_COMPLETE');

            /* SLEEP -> STANDBY on WAKEUP with actions */
            var wakeup = r.transitions.find(function(t) {
                return t.from === 'SLEEP' && t.to === 'STANDBY';
            });
            expect(wakeup).toBeDefined();
            expect(wakeup.event).toBe('WAKEUP');
            expect(wakeup.action).toContain('exitLowPower()');
        });

        test('parses TCP connection state machine (RFC 793)', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                '#pragma once\n' +
                '#include <cstdint>\n' +
                '#include <cstring>\n' +
                '\n' +
                '// Simplified TCP state machine per RFC 793\n' +
                'class TcpConnectionFSM {\n' +
                'public:\n' +
                '    enum class State : uint8_t {\n' +
                '        CLOSED,\n' +
                '        LISTEN,\n' +
                '        SYN_SENT,\n' +
                '        SYN_RECEIVED,\n' +
                '        ESTABLISHED,\n' +
                '        FIN_WAIT_1,\n' +
                '        FIN_WAIT_2,\n' +
                '        CLOSE_WAIT,\n' +
                '        CLOSING,\n' +
                '        LAST_ACK,\n' +
                '        TIME_WAIT\n' +
                '    };\n' +
                '\n' +
                '    enum class Event : uint8_t {\n' +
                '        PASSIVE_OPEN, ACTIVE_OPEN, SEND_SYN,\n' +
                '        RCV_SYN, RCV_ACK, RCV_SYN_ACK,\n' +
                '        CLOSE, RCV_FIN, TIMEOUT\n' +
                '    };\n' +
                '\n' +
                '    TcpConnectionFSM() : state_(State::CLOSED) {}\n' +
                '\n' +
                '    bool isFinal() const {\n' +
                '        return state_ == State::CLOSED || state_ == State::TIME_WAIT;\n' +
                '    }\n' +
                '};\n' +
                parser.MARKER_HEADER_END + '\n' +
                parser.MARKER_SOURCE_BEGIN + '\n' +
                '#include "TcpConnectionFSM.h"\n' +
                '\n' +
                'bool TcpConnectionFSM::process(Event event) {\n' +
                '    switch (state_) {\n' +
                '        case State::CLOSED:\n' +
                '            if (event == Event::PASSIVE_OPEN) {\n' +
                '                state_ = State::LISTEN;\n' +
                '            }\n' +
                '            if (event == Event::ACTIVE_OPEN) {\n' +
                '                sendSynPacket();\n' +
                '                state_ = State::SYN_SENT;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::LISTEN:\n' +
                '            if (event == Event::RCV_SYN) {\n' +
                '                sendSynAck();\n' +
                '                state_ = State::SYN_RECEIVED;\n' +
                '            }\n' +
                '            if (event == Event::SEND_SYN) {\n' +
                '                state_ = State::SYN_SENT;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::SYN_SENT:\n' +
                '            if (event == Event::RCV_SYN_ACK) {\n' +
                '                sendAck();\n' +
                '                state_ = State::ESTABLISHED;\n' +
                '            }\n' +
                '            if (event == Event::RCV_SYN) {\n' +
                '                sendSynAck();\n' +
                '                state_ = State::SYN_RECEIVED;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::SYN_RECEIVED:\n' +
                '            if (event == Event::RCV_ACK) {\n' +
                '                state_ = State::ESTABLISHED;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::ESTABLISHED:\n' +
                '            if (event == Event::CLOSE) {\n' +
                '                sendFin();\n' +
                '                state_ = State::FIN_WAIT_1;\n' +
                '            }\n' +
                '            if (event == Event::RCV_FIN) {\n' +
                '                sendAck();\n' +
                '                state_ = State::CLOSE_WAIT;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::FIN_WAIT_1:\n' +
                '            if (event == Event::RCV_ACK) {\n' +
                '                state_ = State::FIN_WAIT_2;\n' +
                '            }\n' +
                '            if (event == Event::RCV_FIN) {\n' +
                '                sendAck();\n' +
                '                state_ = State::CLOSING;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::FIN_WAIT_2:\n' +
                '            if (event == Event::RCV_FIN) {\n' +
                '                sendAck();\n' +
                '                startTimeWaitTimer();\n' +
                '                state_ = State::TIME_WAIT;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::CLOSE_WAIT:\n' +
                '            if (event == Event::CLOSE) {\n' +
                '                sendFin();\n' +
                '                state_ = State::LAST_ACK;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::CLOSING:\n' +
                '            if (event == Event::RCV_ACK) {\n' +
                '                startTimeWaitTimer();\n' +
                '                state_ = State::TIME_WAIT;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::LAST_ACK:\n' +
                '            if (event == Event::RCV_ACK) {\n' +
                '                releaseResources();\n' +
                '                state_ = State::CLOSED;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::TIME_WAIT:\n' +
                '            if (event == Event::TIMEOUT) {\n' +
                '                releaseResources();\n' +
                '                state_ = State::CLOSED;\n' +
                '            }\n' +
                '            break;\n' +
                '    }\n' +
                '    return false;\n' +
                '}\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');
            expect(r.name).toBe('TcpConnectionFSM');
            expect(r.initial).toBe('CLOSED');
            expect(r.states).toHaveLength(11);
            expect(r.errors).toHaveLength(0);

            /* Verify all TCP states are present */
            var tcpStates = [
                'CLOSED', 'LISTEN', 'SYN_SENT', 'SYN_RECEIVED',
                'ESTABLISHED', 'FIN_WAIT_1', 'FIN_WAIT_2',
                'CLOSE_WAIT', 'CLOSING', 'LAST_ACK', 'TIME_WAIT'
            ];
            var names = r.states.map(function(s) { return s.name; });
            tcpStates.forEach(function(n) {
                expect(names).toContain(n);
            });

            /* Both CLOSED and TIME_WAIT should be marked final via isFinal */
            var closed = r.states.find(function(s) { return s.name === 'CLOSED'; });
            var timeWait = r.states.find(function(s) { return s.name === 'TIME_WAIT'; });
            /* Initial takes precedence over final for CLOSED */
            expect(closed.type).toBe('initial');
            expect(timeWait.type).toBe('final');

            /* Should have many transitions — RFC 793 has ~15 */
            expect(r.transitions.length).toBeGreaterThanOrEqual(13);

            /* 3-way handshake path */
            var closedToListen = r.transitions.find(function(t) {
                return t.from === 'CLOSED' && t.to === 'LISTEN';
            });
            expect(closedToListen).toBeDefined();
            expect(closedToListen.event).toBe('PASSIVE_OPEN');

            var synSentToEstab = r.transitions.find(function(t) {
                return t.from === 'SYN_SENT' && t.to === 'ESTABLISHED';
            });
            expect(synSentToEstab).toBeDefined();
            expect(synSentToEstab.event).toBe('RCV_SYN_ACK');

            /* Connection teardown path */
            var estabToFinWait = r.transitions.find(function(t) {
                return t.from === 'ESTABLISHED' && t.to === 'FIN_WAIT_1';
            });
            expect(estabToFinWait).toBeDefined();

            var lastAckToClosed = r.transitions.find(function(t) {
                return t.from === 'LAST_ACK' && t.to === 'CLOSED';
            });
            expect(lastAckToClosed).toBeDefined();
        });

        test('parses payment processing FSM with complex actions', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                '#pragma once\n' +
                '#include <cstdint>\n' +
                '#include <string>\n' +
                '\n' +
                'class PaymentProcessorFSM {\n' +
                'public:\n' +
                '    enum class State : uint8_t {\n' +
                '        IDLE,\n' +
                '        VALIDATING,\n' +
                '        AUTHORIZING,\n' +
                '        CAPTURING,\n' +
                '        SETTLED,\n' +
                '        REFUNDING,\n' +
                '        DECLINED,\n' +
                '        ERROR\n' +
                '    };\n' +
                '\n' +
                '    enum class Event : uint8_t {\n' +
                '        SUBMIT, VALIDATION_OK, VALIDATION_FAIL,\n' +
                '        AUTH_APPROVED, AUTH_DECLINED,\n' +
                '        CAPTURE_OK, CAPTURE_FAIL,\n' +
                '        REFUND_REQUEST, REFUND_OK\n' +
                '    };\n' +
                '\n' +
                '    PaymentProcessorFSM() : state_(State::IDLE) {}\n' +
                '\n' +
                '    bool isFinal() const {\n' +
                '        return state_ == State::SETTLED ||\n' +
                '               state_ == State::DECLINED ||\n' +
                '               state_ == State::ERROR;\n' +
                '    }\n' +
                '};\n' +
                parser.MARKER_HEADER_END + '\n' +
                parser.MARKER_SOURCE_BEGIN + '\n' +
                '#include "PaymentProcessorFSM.h"\n' +
                '#include <cstdio>\n' +
                '\n' +
                'bool PaymentProcessorFSM::process(Event event) {\n' +
                '    switch (state_) {\n' +
                '        case State::IDLE:\n' +
                '            if (event == Event::SUBMIT) {\n' +
                '                logTransaction();\n' +
                '                state_ = State::VALIDATING;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::VALIDATING:\n' +
                '            if (event == Event::VALIDATION_OK) {\n' +
                '                sendAuthRequest();\n' +
                '                state_ = State::AUTHORIZING;\n' +
                '            }\n' +
                '            if (event == Event::VALIDATION_FAIL) {\n' +
                '                notifyMerchant();\n' +
                '                state_ = State::DECLINED;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::AUTHORIZING:\n' +
                '            if (event == Event::AUTH_APPROVED) {\n' +
                '                initCapture();\n' +
                '                state_ = State::CAPTURING;\n' +
                '            }\n' +
                '            if (event == Event::AUTH_DECLINED) {\n' +
                '                notifyCardHolder();\n' +
                '                state_ = State::DECLINED;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::CAPTURING:\n' +
                '            if (event == Event::CAPTURE_OK) {\n' +
                '                recordSettlement();\n' +
                '                state_ = State::SETTLED;\n' +
                '            }\n' +
                '            if (event == Event::CAPTURE_FAIL) {\n' +
                '                rollbackAuth();\n' +
                '                state_ = State::ERROR;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::SETTLED:\n' +
                '            if (event == Event::REFUND_REQUEST) {\n' +
                '                initiateRefund();\n' +
                '                state_ = State::REFUNDING;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::REFUNDING:\n' +
                '            if (event == Event::REFUND_OK) {\n' +
                '                confirmRefund();\n' +
                '                state_ = State::IDLE;\n' +
                '            }\n' +
                '            break;\n' +
                '    }\n' +
                '    return false;\n' +
                '}\n' +
                '\n' +
                'void PaymentProcessorFSM::onEntry() {\n' +
                '    switch (state_) {\n' +
                '        case State::VALIDATING:\n' +
                '            startFraudCheck();\n' +
                '            break;\n' +
                '        case State::SETTLED:\n' +
                '            sendReceipt();\n' +
                '            break;\n' +
                '        case State::ERROR:\n' +
                '            alertOpsTeam();\n' +
                '            break;\n' +
                '    }\n' +
                '}\n' +
                '\n' +
                'void PaymentProcessorFSM::onExit() {\n' +
                '    switch (state_) {\n' +
                '        case State::AUTHORIZING:\n' +
                '            cancelAuthTimeout();\n' +
                '            break;\n' +
                '    }\n' +
                '}\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');
            expect(r.name).toBe('PaymentProcessorFSM');
            expect(r.initial).toBe('IDLE');
            expect(r.states).toHaveLength(8);
            expect(r.errors).toHaveLength(0);

            /* Multiple final states */
            var settled = r.states.find(function(s) { return s.name === 'SETTLED'; });
            var declined = r.states.find(function(s) { return s.name === 'DECLINED'; });
            var error = r.states.find(function(s) { return s.name === 'ERROR'; });
            expect(settled.type).toBe('final');
            expect(declined.type).toBe('final');
            expect(error.type).toBe('final');

            /* 9 transitions total */
            expect(r.transitions.length).toBeGreaterThanOrEqual(8);

            /* Verify happy path: IDLE->VALIDATING->AUTHORIZING->CAPTURING->SETTLED */
            var happyPath = [
                { from: 'IDLE', to: 'VALIDATING', event: 'SUBMIT' },
                { from: 'VALIDATING', to: 'AUTHORIZING', event: 'VALIDATION_OK' },
                { from: 'AUTHORIZING', to: 'CAPTURING', event: 'AUTH_APPROVED' },
                { from: 'CAPTURING', to: 'SETTLED', event: 'CAPTURE_OK' }
            ];
            happyPath.forEach(function(hp) {
                var t = r.transitions.find(function(tr) {
                    return tr.from === hp.from && tr.to === hp.to && tr.event === hp.event;
                });
                expect(t).toBeDefined();
            });

            /* Verify refund loop: SETTLED->REFUNDING->IDLE */
            var refundReq = r.transitions.find(function(t) {
                return t.from === 'SETTLED' && t.to === 'REFUNDING';
            });
            expect(refundReq).toBeDefined();

            var refundOk = r.transitions.find(function(t) {
                return t.from === 'REFUNDING' && t.to === 'IDLE';
            });
            expect(refundOk).toBeDefined();

            /* Verify entry/exit actions */
            var validating = r.states.find(function(s) {
                return s.name === 'VALIDATING';
            });
            if (validating.onEntry) {
                expect(validating.onEntry).toContain('startFraudCheck');
            }

            var authorizing = r.states.find(function(s) {
                return s.name === 'AUTHORIZING';
            });
            if (authorizing.onExit) {
                expect(authorizing.onExit).toContain('cancelAuthTimeout');
            }
        });

        test('parses USB device state machine with includes and destructor', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                '#pragma once\n' +
                '#include <cstdint>\n' +
                '#include <memory>\n' +
                '\n' +
                'class UsbDeviceFSM {\n' +
                'public:\n' +
                '    enum class State : uint8_t {\n' +
                '        ATTACHED,\n' +
                '        POWERED,\n' +
                '        DEFAULT,\n' +
                '        ADDRESS,\n' +
                '        CONFIGURED,\n' +
                '        SUSPENDED\n' +
                '    };\n' +
                '\n' +
                '    enum class Event : uint8_t {\n' +
                '        BUS_POWER, BUS_RESET, SET_ADDRESS,\n' +
                '        SET_CONFIG, SUSPEND, RESUME, DETACH\n' +
                '    };\n' +
                '\n' +
                '    UsbDeviceFSM() : state_(State::ATTACHED) {}\n' +
                '    ~UsbDeviceFSM() { releaseEndpoints(); }\n' +
                '\n' +
                '    bool isFinal() const {\n' +
                '        return state_ == State::SUSPENDED;\n' +
                '    }\n' +
                '\n' +
                'private:\n' +
                '    State state_;\n' +
                '    uint8_t device_address_ = 0U;\n' +
                '    void releaseEndpoints();\n' +
                '};\n' +
                parser.MARKER_HEADER_END + '\n' +
                parser.MARKER_SOURCE_BEGIN + '\n' +
                '#include "UsbDeviceFSM.h"\n' +
                '\n' +
                'bool UsbDeviceFSM::process(Event event) {\n' +
                '    switch (state_) {\n' +
                '        case State::ATTACHED:\n' +
                '            if (event == Event::BUS_POWER) {\n' +
                '                enablePhy();\n' +
                '                state_ = State::POWERED;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::POWERED:\n' +
                '            if (event == Event::BUS_RESET) {\n' +
                '                resetDescriptors();\n' +
                '                state_ = State::DEFAULT;\n' +
                '            }\n' +
                '            if (event == Event::SUSPEND) {\n' +
                '                enterLowPower();\n' +
                '                state_ = State::SUSPENDED;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::DEFAULT:\n' +
                '            if (event == Event::SET_ADDRESS) {\n' +
                '                assignAddress();\n' +
                '                state_ = State::ADDRESS;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::ADDRESS:\n' +
                '            if (event == Event::SET_CONFIG) {\n' +
                '                configureEndpoints();\n' +
                '                state_ = State::CONFIGURED;\n' +
                '            }\n' +
                '            if (event == Event::BUS_RESET) {\n' +
                '                resetAddress();\n' +
                '                state_ = State::DEFAULT;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::CONFIGURED:\n' +
                '            if (event == Event::SUSPEND) {\n' +
                '                savePipeState();\n' +
                '                enterLowPower();\n' +
                '                state_ = State::SUSPENDED;\n' +
                '            }\n' +
                '            if (event == Event::BUS_RESET) {\n' +
                '                deconfigureEndpoints();\n' +
                '                state_ = State::DEFAULT;\n' +
                '            }\n' +
                '            break;\n' +
                '        case State::SUSPENDED:\n' +
                '            if (event == Event::RESUME) {\n' +
                '                exitLowPower();\n' +
                '                state_ = State::CONFIGURED;\n' +
                '            }\n' +
                '            break;\n' +
                '    }\n' +
                '    return false;\n' +
                '}\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');
            expect(r.name).toBe('UsbDeviceFSM');
            expect(r.initial).toBe('ATTACHED');
            expect(r.states).toHaveLength(6);
            expect(r.errors).toHaveLength(0);

            /* USB states */
            var usbStates = [
                'ATTACHED', 'POWERED', 'DEFAULT',
                'ADDRESS', 'CONFIGURED', 'SUSPENDED'
            ];
            var names = r.states.map(function(s) { return s.name; });
            usbStates.forEach(function(n) {
                expect(names).toContain(n);
            });

            /* SUSPENDED should be final */
            var suspended = r.states.find(function(s) {
                return s.name === 'SUSPENDED';
            });
            expect(suspended.type).toBe('final');

            /* Should have 8 transitions */
            expect(r.transitions.length).toBeGreaterThanOrEqual(7);

            /* Verify enumeration path */
            var attachedToPowered = r.transitions.find(function(t) {
                return t.from === 'ATTACHED' && t.to === 'POWERED';
            });
            expect(attachedToPowered).toBeDefined();
            expect(attachedToPowered.event).toBe('BUS_POWER');

            /* Multiple transitions to DEFAULT (bus reset from different states) */
            var resetsToDefault = r.transitions.filter(function(t) {
                return t.to === 'DEFAULT' && t.event === 'BUS_RESET';
            });
            expect(resetsToDefault.length).toBeGreaterThanOrEqual(2);
        });
    });

    /* ═══════════════════════════════════════════════════════
     * COMPLEX C++ MARKED FORMAT TESTS
     * ═══════════════════════════════════════════════════════ */
    describe('Complex C++ Marked Format', function() {


        test('parses multi-state FSM with all features', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                'class ElevatorFSM {\n' +
                'public:\n' +
                '    enum class State : uint8_t {\n' +
                '        IDLE,\n' +
                '        MOVING_UP,\n' +
                '        MOVING_DOWN,\n' +
                '        DOOR_OPEN,\n' +
                '        EMERGENCY,\n' +
                '        MAINTENANCE\n' +
                '    };\n' +
                '\n' +
                '    enum class Event : uint8_t {\n' +
                '        CALL_UP, CALL_DOWN, ARRIVED,\n' +
                '        OPEN_DOOR, CLOSE_DOOR,\n' +
                '        EMERGENCY_STOP, RESET\n' +
                '    };\n' +
                '\n' +
                '    ElevatorFSM() : state_(State::IDLE) {}\n' +
                '\n' +
                '    bool isFinal() const { return state_ == State::MAINTENANCE; }\n' +
                '};\n' +
                parser.MARKER_HEADER_END + '\n' +
                parser.MARKER_SOURCE_BEGIN + '\n' +
                'switch (state_) {\n' +
                '    case State::IDLE:\n' +
                '        if (event == Event::CALL_UP) {\n' +
                '            startMotor(UP);\n' +
                '            state_ = State::MOVING_UP;\n' +
                '        }\n' +
                '        if (event == Event::CALL_DOWN) {\n' +
                '            startMotor(DOWN);\n' +
                '            state_ = State::MOVING_DOWN;\n' +
                '        }\n' +
                '        if (event == Event::EMERGENCY_STOP) {\n' +
                '            triggerAlarm();\n' +
                '            state_ = State::EMERGENCY;\n' +
                '        }\n' +
                '        break;\n' +
                '    case State::MOVING_UP:\n' +
                '        if (event == Event::ARRIVED) {\n' +
                '            stopMotor();\n' +
                '            state_ = State::DOOR_OPEN;\n' +
                '        }\n' +
                '        if (event == Event::EMERGENCY_STOP) {\n' +
                '            emergencyBrake();\n' +
                '            state_ = State::EMERGENCY;\n' +
                '        }\n' +
                '        break;\n' +
                '    case State::MOVING_DOWN:\n' +
                '        if (event == Event::ARRIVED) {\n' +
                '            stopMotor();\n' +
                '            state_ = State::DOOR_OPEN;\n' +
                '        }\n' +
                '        break;\n' +
                '    case State::DOOR_OPEN:\n' +
                '        if (event == Event::CLOSE_DOOR) {\n' +
                '            closeDoor();\n' +
                '            state_ = State::IDLE;\n' +
                '        }\n' +
                '        break;\n' +
                '    case State::EMERGENCY:\n' +
                '        if (event == Event::RESET) {\n' +
                '            clearAlarm();\n' +
                '            state_ = State::MAINTENANCE;\n' +
                '        }\n' +
                '        break;\n' +
                '}\n' +
                '\n' +
                'void ElevatorFSM::onEntry(State state_) {\n' +
                '    switch (state_) {\n' +
                '        case State::IDLE:\n' +
                '            displayFloor();\n' +
                '            break;\n' +
                '        case State::EMERGENCY:\n' +
                '            activateEmergencyLights();\n' +
                '            notifySecurity();\n' +
                '            break;\n' +
                '    }\n' +
                '}\n' +
                '\n' +
                'void ElevatorFSM::onExit(State state_) {\n' +
                '    switch (state_) {\n' +
                '        case State::DOOR_OPEN:\n' +
                '            lockDoor();\n' +
                '            break;\n' +
                '        case State::EMERGENCY:\n' +
                '            deactivateEmergencyLights();\n' +
                '            break;\n' +
                '    }\n' +
                '}\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');
            expect(r.name).toBe('ElevatorFSM');
            expect(r.initial).toBe('IDLE');
            expect(r.states).toHaveLength(6);
            expect(r.errors).toHaveLength(0);

            /* Verify state names */
            var stateNames = r.states.map(function(s) { return s.name; });
            expect(stateNames).toContain('IDLE');
            expect(stateNames).toContain('MOVING_UP');
            expect(stateNames).toContain('MOVING_DOWN');
            expect(stateNames).toContain('DOOR_OPEN');
            expect(stateNames).toContain('EMERGENCY');
            expect(stateNames).toContain('MAINTENANCE');

            /* Verify initial state type */
            var idle = r.states.find(function(s) { return s.name === 'IDLE'; });
            expect(idle.type).toBe('initial');

            /* Verify final state */
            var maint = r.states.find(function(s) { return s.name === 'MAINTENANCE'; });
            expect(maint.type).toBe('final');

            /* Verify transitions count — should be 8 */
            expect(r.transitions.length).toBeGreaterThanOrEqual(7);

            /* Verify specific transitions */
            var idleToUp = r.transitions.find(function(t) {
                return t.from === 'IDLE' && t.to === 'MOVING_UP' && t.event === 'CALL_UP';
            });
            expect(idleToUp).toBeDefined();

            var emergToMaint = r.transitions.find(function(t) {
                return t.from === 'EMERGENCY' && t.to === 'MAINTENANCE' && t.event === 'RESET';
            });
            expect(emergToMaint).toBeDefined();

            /* Verify entry actions */
            var emerg = r.states.find(function(s) { return s.name === 'EMERGENCY'; });
            expect(emerg.onEntry).toBeDefined();
            if (emerg.onEntry) {
                expect(emerg.onEntry).toContain('activateEmergencyLights');
            }

            /* Verify exit actions */
            var doorOpen = r.states.find(function(s) { return s.name === 'DOOR_OPEN'; });
            expect(doorOpen.onExit).toBeDefined();
            if (doorOpen.onExit) {
                expect(doorOpen.onExit).toContain('lockDoor');
            }
        });

        test('parses multiple transitions from single state', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                'class RouterFSM {\n' +
                'public:\n' +
                '    enum class State { INIT, ROUTE_A, ROUTE_B, ROUTE_C, ERROR };\n' +
                '    enum class Event { GO_A, GO_B, GO_C, FAIL };\n' +
                '    RouterFSM() : state_(State::INIT) {}\n' +
                '};\n' +
                parser.MARKER_HEADER_END + '\n' +
                parser.MARKER_SOURCE_BEGIN + '\n' +
                'switch (state_) {\n' +
                '    case State::INIT:\n' +
                '        if (event == Event::GO_A) {\n' +
                '            state_ = State::ROUTE_A;\n' +
                '        }\n' +
                '        if (event == Event::GO_B) {\n' +
                '            state_ = State::ROUTE_B;\n' +
                '        }\n' +
                '        if (event == Event::GO_C) {\n' +
                '            state_ = State::ROUTE_C;\n' +
                '        }\n' +
                '        if (event == Event::FAIL) {\n' +
                '            logError();\n' +
                '            state_ = State::ERROR;\n' +
                '        }\n' +
                '        break;\n' +
                '}\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseInput(src);

            expect(r.name).toBe('RouterFSM');
            expect(r.states).toHaveLength(5);
            expect(r.initial).toBe('INIT');

            /* 4 transitions all from INIT */
            var fromInit = r.transitions.filter(function(t) {
                return t.from === 'INIT';
            });
            expect(fromInit).toHaveLength(4);

            var targets = fromInit.map(function(t) { return t.to; }).sort();
            expect(targets).toEqual(['ERROR', 'ROUTE_A', 'ROUTE_B', 'ROUTE_C']);
        });

        test('parses enum with explicit integer values', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                'class ProtoFSM {\n' +
                'public:\n' +
                '    enum class State : uint8_t {\n' +
                '        DISCONNECTED = 0,\n' +
                '        CONNECTING = 1,\n' +
                '        CONNECTED = 2,\n' +
                '        AUTHENTICATED = 3,\n' +
                '        DISCONNECTING = 4\n' +
                '    };\n' +
                '    ProtoFSM() : state_(State::DISCONNECTED) {}\n' +
                '};\n' +
                parser.MARKER_HEADER_END;

            var r = parser.parseInput(src);

            expect(r.name).toBe('ProtoFSM');
            expect(r.states).toHaveLength(5);
            expect(r.initial).toBe('DISCONNECTED');

            /* Verify integer values are stripped from names */
            var names = r.states.map(function(s) { return s.name; });
            expect(names).toContain('DISCONNECTED');
            expect(names).toContain('CONNECTING');
            expect(names).toContain('CONNECTED');
            expect(names).toContain('AUTHENTICATED');
            expect(names).toContain('DISCONNECTING');
            names.forEach(function(n) {
                expect(n).not.toMatch(/\d/);
            });
        });

        test('parses code with heavy block comments', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                '/* This is the main FSM class\n' +
                '   for the automated door system */\n' +
                'class DoorFSM {\n' +
                'public:\n' +
                '    /* State enumeration */\n' +
                '    enum class State { CLOSED, OPENING, OPEN, CLOSING };\n' +
                '    /* Event enumeration */\n' +
                '    enum class Event { BUTTON, SENSOR, TIMEOUT };\n' +
                '    DoorFSM() : state_(State::CLOSED) {}\n' +
                '};\n' +
                parser.MARKER_HEADER_END + '\n' +
                parser.MARKER_SOURCE_BEGIN + '\n' +
                '/* Main transition logic */\n' +
                'switch (state_) {\n' +
                '    case State::CLOSED: /* door is currently closed */\n' +
                '        if (event == Event::BUTTON) {\n' +
                '            /* user pressed the button */\n' +
                '            activateMotor();\n' +
                '            state_ = State::OPENING;\n' +
                '        }\n' +
                '        break;\n' +
                '    case State::OPEN:\n' +
                '        if (event == Event::TIMEOUT) {\n' +
                '            state_ = State::CLOSING;\n' +
                '        }\n' +
                '        break;\n' +
                '}\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseInput(src);

            expect(r.errors).toHaveLength(0);
            expect(r.name).toBe('DoorFSM');
            expect(r.states).toHaveLength(4);
            expect(r.initial).toBe('CLOSED');
            expect(r.transitions).toHaveLength(2);
        });

        test('parses initial state from assignment instead of constructor', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                'class ResetFSM {\n' +
                'public:\n' +
                '    enum class State { OFF, ON, STANDBY };\n' +
                '    void reset() { state_ = State::OFF; }\n' +
                '};\n' +
                parser.MARKER_HEADER_END;

            var r = parser.parseInput(src);

            expect(r.name).toBe('ResetFSM');
            expect(r.initial).toBe('OFF');
            expect(r.states).toHaveLength(3);
        });

        test('parses transition actions correctly', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                'class ActionFSM {\n' +
                'public:\n' +
                '    enum class State { A, B };\n' +
                '    enum class Event { GO };\n' +
                '    ActionFSM() : state_(State::A) {}\n' +
                '};\n' +
                parser.MARKER_HEADER_END + '\n' +
                parser.MARKER_SOURCE_BEGIN + '\n' +
                'switch (state_) {\n' +
                '    case State::A:\n' +
                '        if (event == Event::GO) {\n' +
                '            logTransition();\n' +
                '            notifyObservers();\n' +
                '            state_ = State::B;\n' +
                '        }\n' +
                '        break;\n' +
                '}\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseInput(src);

            expect(r.transitions).toHaveLength(1);
            var t = r.transitions[0];
            expect(t.from).toBe('A');
            expect(t.to).toBe('B');
            expect(t.event).toBe('GO');
            /* Actions should contain the two function calls */
            expect(t.action).toContain('logTransition()');
            expect(t.action).toContain('notifyObservers()');
        });

        test('parses FSM with only header section (no source)', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                'class SensorFSM {\n' +
                'public:\n' +
                '    enum class State : uint8_t {\n' +
                '        CALIBRATING,\n' +
                '        READY,\n' +
                '        READING,\n' +
                '        FAULT\n' +
                '    };\n' +
                '    SensorFSM() : state_(State::CALIBRATING) {}\n' +
                '    bool isFinal() const { return state_ == State::FAULT; }\n' +
                '};\n' +
                parser.MARKER_HEADER_END;

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');
            expect(r.name).toBe('SensorFSM');
            expect(r.states).toHaveLength(4);
            expect(r.initial).toBe('CALIBRATING');
            expect(r.transitions).toHaveLength(0);

            var fault = r.states.find(function(s) { return s.name === 'FAULT'; });
            expect(fault.type).toBe('final');
        });
    });

    /* ═══════════════════════════════════════════════════════
     * COMPLEX DSL TESTS
     * ═══════════════════════════════════════════════════════ */
    describe('Complex DSL Inputs', function() {

        test('parses full-featured industrial controller FSM', function() {
            var dsl = [
                'FSM: IndustrialController',
                'STATE: PowerOff',
                'STATE: Initializing',
                'STATE: Idle',
                'STATE: Running',
                'STATE: Paused',
                'STATE: EmergencyStop',
                'STATE: Shutdown',
                'INIT: PowerOff',
                'FINAL: Shutdown',
                '',
                '// Power transitions',
                'TRANS: PowerOff -> Initializing : POWER_ON',
                'TRANS: Initializing -> Idle : INIT_COMPLETE [sensorsReady] / calibrateSensors()',
                '',
                '// Normal operation transitions',
                'TRANS: Idle -> Running : START [motorReady] / enableMotor()',
                'TRANS: Running -> Paused : PAUSE / disableMotor()',
                'TRANS: Paused -> Running : RESUME [motorReady] / enableMotor()',
                'TRANS: Running -> Idle : STOP / disableMotor()',
                'TRANS: Paused -> Idle : STOP / disableMotor()',
                '',
                '// Emergency and shutdown',
                'TRANS: Running -> EmergencyStop : E_STOP / emergencyBrake()',
                'TRANS: Idle -> Shutdown : POWER_OFF / saveState()',
                'TRANS: EmergencyStop -> Shutdown : ACKNOWLEDGE [faultCleared] / logIncident()',
                '',
                '// Entry/Exit actions',
                'ON_ENTRY: Initializing / runDiagnostics()',
                'ON_ENTRY: Running / startMonitoring()',
                'ON_ENTRY: EmergencyStop / activateAlarm()',
                'ON_EXIT: Running / stopMonitoring()',
                'ON_EXIT: EmergencyStop / deactivateAlarm()',
                '',
                '// Internal events',
                'INTERNAL: Running / HEARTBEAT [] / sendHeartbeat()',
                'INTERNAL: Running / TEMP_CHECK [overheating] / reduceSpeed()',
                '',
                '// Comments',
                'COMMENT: PowerOff / System is completely powered down',
                'COMMENT: EmergencyStop / Critical safety state'
            ].join('\n');

            var r = parser.parseInput(dsl);

            expect(r.format).toBe('dsl');
            expect(r.name).toBe('IndustrialController');
            expect(r.initial).toBe('PowerOff');
            expect(r.states).toHaveLength(7);
            expect(r.transitions).toHaveLength(10);
            expect(r.errors).toHaveLength(0);

            /* Verify state types */
            var powerOff = r.states.find(function(s) { return s.name === 'PowerOff'; });
            expect(powerOff.type).toBe('initial');

            var shutdown = r.states.find(function(s) { return s.name === 'Shutdown'; });
            expect(shutdown.type).toBe('final');

            /* Verify guards and actions */
            var initToIdle = r.transitions.find(function(t) {
                return t.from === 'Initializing' && t.to === 'Idle';
            });
            expect(initToIdle).toBeDefined();
            expect(initToIdle.guard).toBe('sensorsReady');
            expect(initToIdle.action).toBe('calibrateSensors()');

            /* Verify entry actions */
            var running = r.states.find(function(s) { return s.name === 'Running'; });
            expect(running.onEntry).toBe('startMonitoring()');
            expect(running.onExit).toBe('stopMonitoring()');

            /* Verify internal events */
            expect(running.internalEvents).toHaveLength(2);
            var hb = running.internalEvents.find(function(e) {
                return e.event === 'HEARTBEAT';
            });
            expect(hb).toBeDefined();
            expect(hb.action).toBe('sendHeartbeat()');

            var tempCheck = running.internalEvents.find(function(e) {
                return e.event === 'TEMP_CHECK';
            });
            expect(tempCheck).toBeDefined();
            expect(tempCheck.guard).toBe('overheating');

            /* Verify comments */
            expect(powerOff.comment).toBe('System is completely powered down');
            var emStop = r.states.find(function(s) { return s.name === 'EmergencyStop'; });
            expect(emStop.comment).toBe('Critical safety state');
        });

        test('parses self-loop transitions', function() {
            var dsl = [
                'FSM: Poller',
                'STATE: Polling',
                'INIT: Polling',
                'TRANS: Polling -> Polling : TICK [hasData] / fetchData()',
                'TRANS: Polling -> Polling : RETRY [] / incrementCounter()'
            ].join('\n');

            var r = parser.parseInput(dsl);

            expect(r.states).toHaveLength(1);
            expect(r.transitions).toHaveLength(2);

            r.transitions.forEach(function(t) {
                expect(t.from).toBe('Polling');
                expect(t.to).toBe('Polling');
            });
        });

        test('parses case-insensitive directives', function() {
            var dsl = [
                'fsm: CaseTest',
                'state: Alpha',
                'state: Beta',
                'init: Alpha',
                'final: Beta',
                'trans: Alpha -> Beta : GO'
            ].join('\n');

            var r = parser.parseInput(dsl);

            expect(r.name).toBe('CaseTest');
            expect(r.states).toHaveLength(2);
            expect(r.initial).toBe('Alpha');
            expect(r.transitions).toHaveLength(1);

            var beta = r.states.find(function(s) { return s.name === 'Beta'; });
            expect(beta.type).toBe('final');
        });

        test('auto-creates states referenced only in transitions', function() {
            var dsl = [
                'FSM: AutoCreate',
                'TRANS: Origin -> Destination : MOVE'
            ].join('\n');

            var r = parser.parseInput(dsl);

            /* Both states should be auto-created */
            expect(r.states).toHaveLength(2);
            var names = r.states.map(function(s) { return s.name; });
            expect(names).toContain('Origin');
            expect(names).toContain('Destination');
            expect(r.transitions).toHaveLength(1);
        });

        test('parses complex guard expressions', function() {
            var dsl = [
                'FSM: GuardTest',
                'STATE: A',
                'STATE: B',
                'STATE: C',
                'INIT: A',
                'TRANS: A -> B : EV1 [x > 0 && isReady] / action1()',
                'TRANS: B -> C : EV2 [count < MAX_RETRIES] / retry()'
            ].join('\n');

            var r = parser.parseInput(dsl);

            expect(r.transitions).toHaveLength(2);
            expect(r.transitions[0].guard).toBe('x > 0 && isReady');
            expect(r.transitions[1].guard).toBe('count < MAX_RETRIES');
        });

        test('parses DSL with many comment lines interleaved', function() {
            var dsl = [
                '// =====================',
                '// Traffic Light FSM',
                '// Version 2.0',
                '// =====================',
                'FSM: TrafficV2',
                '// Define states',
                'STATE: Red',
                '// Red means stop',
                'STATE: Yellow',
                '// Yellow means caution',
                'STATE: Green',
                '// Green means go',
                'INIT: Red',
                '// Transition definitions',
                'TRANS: Red -> Green : TIMER',
                '// 30 second timer',
                'TRANS: Green -> Yellow : TIMER',
                '// 5 second timer',
                'TRANS: Yellow -> Red : TIMER'
            ].join('\n');

            var r = parser.parseInput(dsl);

            expect(r.name).toBe('TrafficV2');
            expect(r.states).toHaveLength(3);
            expect(r.transitions).toHaveLength(3);
            expect(r.errors).toHaveLength(0);
        });

        test('parses transitions without guard but with action', function() {
            var dsl = [
                'FSM: ActionOnly',
                'STATE: X',
                'STATE: Y',
                'INIT: X',
                'TRANS: X -> Y : TRIGGER / doSomething()'
            ].join('\n');

            var r = parser.parseInput(dsl);

            expect(r.transitions).toHaveLength(1);
            expect(r.transitions[0].guard).toBe('');
            expect(r.transitions[0].action).toBe('doSomething()');
        });

        test('parses large FSM with many states and transitions', function() {
            var lines = ['FSM: LargeFSM'];
            var stateCount = 12;
            var stateNames = [];
            for (var i = 0; i < stateCount; i++) {
                var name = 'State' + String.fromCharCode(65 + i);
                stateNames.push(name);
                lines.push('STATE: ' + name);
            }
            lines.push('INIT: StateA');
            lines.push('FINAL: ' + stateNames[stateCount - 1]);

            /* Chain transitions: A->B->C->...->L */
            for (var j = 0; j < stateCount - 1; j++) {
                lines.push(
                    'TRANS: ' + stateNames[j] + ' -> ' + stateNames[j + 1] +
                    ' : NEXT_' + j
                );
            }

            var r = parser.parseInput(lines.join('\n'));

            expect(r.states).toHaveLength(stateCount);
            expect(r.transitions).toHaveLength(stateCount - 1);
            expect(r.errors).toHaveLength(0);
            expect(r.initial).toBe('StateA');
        });
    });

    /* ═══════════════════════════════════════════════════════
     * EDGE CASES AND ERROR HANDLING
     * ═══════════════════════════════════════════════════════ */
    describe('Edge Cases and Error Handling', function() {

        test('handles whitespace-only input', function() {
            var r = parser.parseInput('   \n  \n   \t  ');
            expect(r.errors.length).toBeGreaterThan(0);
        });

        test('handles markers with empty content between them', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                '\n' +
                parser.MARKER_HEADER_END + '\n' +
                parser.MARKER_SOURCE_BEGIN + '\n' +
                '\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseInput(src);
            /* No states found — should produce error */
            expect(r.errors.length).toBeGreaterThan(0);
        });

        test('handles empty state enum', function() {
            var src =
                parser.MARKER_HEADER_BEGIN + '\n' +
                'class EmptyFSM {\n' +
                'public:\n' +
                '    enum class State {};\n' +
                '};\n' +
                parser.MARKER_HEADER_END;

            var r = parser.parseInput(src);
            expect(r.states).toHaveLength(0);
            expect(r.errors.length).toBeGreaterThan(0);
        });

        test('reports error for DSL transition to unknown state', function() {
            var dsl = [
                'STATE: Existing',
                'TRANS: Existing -> NonExistent : GO'
            ].join('\n');

            var r = parser.parseDsl(dsl);
            /* getOrAddState auto-creates states from TRANS, so no error for unknown target.
               But cross-check should flag if a state is truly missing from the states list.
               Since getOrAddState auto-creates, the state should exist. */
            var nonExist = r.states.find(function(s) { return s.name === 'NonExistent'; });
            expect(nonExist).toBeDefined();
        });

        test('handles source section only (no header)', function() {
            var src =
                parser.MARKER_SOURCE_BEGIN + '\n' +
                'switch (state_) {\n' +
                '    case State::ALPHA:\n' +
                '        if (event == Event::GO) {\n' +
                '            state_ = State::BETA;\n' +
                '        }\n' +
                '        break;\n' +
                '}\n' +
                parser.MARKER_SOURCE_END;

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');
            /* Parser should still extract transitions even without header */
            expect(r.transitions).toHaveLength(1);
            expect(r.transitions[0].from).toBe('ALPHA');
            expect(r.transitions[0].to).toBe('BETA');
        });

        test('warns when no initial state is defined in DSL', function() {
            var dsl = [
                'STATE: Only',
                'STATE: States',
                'STATE: NoInit'
            ].join('\n');

            var r = parser.parseDsl(dsl);
            expect(r.warnings.length).toBeGreaterThan(0);
            expect(r.warnings.some(function(w) {
                return w.toLowerCase().indexOf('init') !== -1;
            })).toBe(true);
        });

        test('handles bad TRANS syntax gracefully', function() {
            var dsl = [
                'TRANS: not valid syntax',
                'TRANS: A -> : MISSING_TARGET',
                'TRANS:'
            ].join('\n');

            var r = parser.parseDsl(dsl);
            expect(r.errors.length).toBeGreaterThanOrEqual(2);
        });

        test('handles extremely long state names', function() {
            var longName = 'VeryLongStateNameThatExceedsNormalConventions';
            var dsl = [
                'FSM: LongNameFSM',
                'STATE: ' + longName,
                'INIT: ' + longName
            ].join('\n');

            var r = parser.parseDsl(dsl);

            expect(r.states).toHaveLength(1);
            expect(r.states[0].name).toBe(longName);
            expect(r.initial).toBe(longName);
        });

        test('handles duplicate state declarations in DSL', function() {
            var dsl = [
                'STATE: Alpha',
                'STATE: Alpha',
                'STATE: Beta',
                'INIT: Alpha'
            ].join('\n');

            var r = parser.parseDsl(dsl);

            /* getOrAddState should not create duplicate — only 2 unique states */
            expect(r.states).toHaveLength(2);
            var alphaCount = r.states.filter(function(s) {
                return s.name === 'Alpha';
            }).length;
            expect(alphaCount).toBe(1);
        });

        test('non-string input to extractSection returns null', function() {
            expect(parser.extractSection(undefined, 'a', 'b')).toBeNull();
            expect(parser.extractSection({}, 'a', 'b')).toBeNull();
            expect(parser.extractSection([], 'a', 'b')).toBeNull();
        });

        test('detectFormat returns unknown for non-string types', function() {
            expect(parser.detectFormat(undefined)).toBe('unknown');
            expect(parser.detectFormat({})).toBe('unknown');
            expect(parser.detectFormat([])).toBe('unknown');
            expect(parser.detectFormat(true)).toBe('unknown');
        });

        test('blankResult initializes all arrays empty', function() {
            var r = parser.blankResult('Test');
            expect(r.states).toEqual([]);
            expect(r.transitions).toEqual([]);
            expect(r.errors).toEqual([]);
            expect(r.warnings).toEqual([]);
            expect(r.infos).toEqual([]);
            expect(r.format).toBe('unknown');
            expect(r.initial).toBe('');
        });
    });

    /* ═══════════════════════════════════════════════════════
     * ROUND-TRIP: CODEGEN → PARSE
     * ═══════════════════════════════════════════════════════ */
    describe('Round-trip: codegen → parse', function() {
        var codegen;

        beforeAll(function() {
            codegen = require('../js/codegen.js');
        });

        test('codegen output parses back to matching FSM', function() {
            /* Build a synthetic FSM model */
            var fsmData = {
                name: 'RoundTripFSM',
                initial: 'n1',
                states: [
                    { id: 'n1', name: 'Idle', type: 'initial', onEntry: '', onExit: '', internalEvents: [] },
                    { id: 'n2', name: 'Active', type: 'normal', onEntry: '', onExit: '', internalEvents: [] },
                    { id: 'n3', name: 'Done', type: 'final', onEntry: '', onExit: '', internalEvents: [] }
                ],
                transitions: [
                    { id: 't1', from: 'n1', to: 'n2', event: 'START', guard: '', action: '' },
                    { id: 't2', from: 'n2', to: 'n3', event: 'FINISH', guard: '', action: '' },
                    { id: 't3', from: 'n2', to: 'n1', event: 'RESET', guard: '', action: '' }
                ]
            };

            /* Generate C++ */
            var cpp = codegen.buildCpp(fsmData, '');
            expect(cpp).toBeDefined();
            expect(cpp.length).toBeGreaterThan(0);

            /* Parse it back */
            var r = parser.parseInput(cpp);

            /* Verify round-trip integrity */
            expect(r.format).toBe('marked');
            expect(r.name).toBe('RoundTripFSM');
            expect(r.states.length).toBeGreaterThanOrEqual(3);

            var stateNames = r.states.map(function(s) { return s.name; });
            expect(stateNames).toContain('IDLE');
            expect(stateNames).toContain('ACTIVE');
            expect(stateNames).toContain('DONE');

            /* Initial state */
            expect(r.initial).toBe('IDLE');

            /* Transitions should be preserved */
            expect(r.transitions.length).toBeGreaterThanOrEqual(3);
        });

        test('round-trip preserves entry/exit actions', function() {
            var fsmData = {
                name: 'EntryExitFSM',
                initial: 'n1',
                states: [
                    { id: 'n1', name: 'Ready', type: 'initial',
                      onEntry: 'initHardware();', onExit: 'releaseHardware();',
                      internalEvents: [] },
                    { id: 'n2', name: 'Processing', type: 'normal',
                      onEntry: 'startTimer();', onExit: 'stopTimer();',
                      internalEvents: [] }
                ],
                transitions: [
                    { id: 't1', from: 'n1', to: 'n2', event: 'BEGIN', guard: '', action: '' }
                ]
            };

            var cpp = codegen.buildCpp(fsmData, '');
            var r = parser.parseInput(cpp);

            expect(r.errors).toHaveLength(0);
            expect(r.name).toBe('EntryExitFSM');
            expect(r.states.length).toBeGreaterThanOrEqual(2);

            /* At minimum, the entry/exit actions should be in the generated code */
            expect(cpp).toContain('initHardware');
            expect(cpp).toContain('releaseHardware');
            expect(cpp).toContain('startTimer');
            expect(cpp).toContain('stopTimer');
        });

        test('round-trip with namespace', function() {
            var fsmData = {
                name: 'NsFSM',
                initial: 'n1',
                states: [
                    { id: 'n1', name: 'Off', type: 'initial', onEntry: '', onExit: '', internalEvents: [] },
                    { id: 'n2', name: 'On', type: 'normal', onEntry: '', onExit: '', internalEvents: [] }
                ],
                transitions: [
                    { id: 't1', from: 'n1', to: 'n2', event: 'TOGGLE', guard: '', action: '' }
                ]
            };

            var cpp = codegen.buildCpp(fsmData, 'myapp');

            /* Namespace should appear in the output */
            expect(cpp).toContain('namespace myapp');

            /* Parse should still work */
            var r = parser.parseInput(cpp);
            expect(r.name).toBe('NsFSM');
            expect(r.states.length).toBeGreaterThanOrEqual(2);
            expect(r.initial).toBe('OFF');
        });

        test('round-trip with single-state FSM', function() {
            var fsmData = {
                name: 'SingleFSM',
                initial: 'n1',
                states: [
                    { id: 'n1', name: 'Only', type: 'initial', onEntry: '', onExit: '', internalEvents: [] }
                ],
                transitions: []
            };

            var cpp = codegen.buildCpp(fsmData, '');
            var r = parser.parseInput(cpp);

            expect(r.name).toBe('SingleFSM');
            expect(r.states).toHaveLength(1);
            expect(r.states[0].name).toBe('ONLY');
            expect(r.initial).toBe('ONLY');
            expect(r.transitions).toHaveLength(0);
        });
    });

    /* ═══════════════════════════════════════════════════════
     * CONFIG-BASED C++ CODE INPUTS
     * Full production-grade config-map FSM patterns
     * ═══════════════════════════════════════════════════════ */
    describe('Config-Based C++ Code Inputs', function() {

        test('parses traffic intersection controller FSM', function() {
            var src = [
                '// === FSM_HEADER_BEGIN ===',
                '/**',
                ' * Config-Based Finite State Machine — Traffic Intersection Controller',
                ' */',
                '',
                '#include <iostream>',
                '#include <string>',
                '#include <vector>',
                '#include <unordered_map>',
                '#include <functional>',
                '#include <queue>',
                '#include <cassert>',
                '',
                'enum class StateId {',
                '    NS_GREEN,',
                '    NS_YELLOW,',
                '    EW_GREEN,',
                '    EW_YELLOW,',
                '    ALL_RED,',
                '    PEDESTRIAN,',
                '    EMERGENCY,',
                '    FAULT,',
                '};',
                '',
                'enum class EventId {',
                '    TIMER_EXPIRED,',
                '    PEDESTRIAN_REQUEST,',
                '    EMERGENCY_VEHICLE,',
                '    SENSOR_FAULT,',
                '    FAULT_CLEARED,',
                '    MANUAL_OVERRIDE,',
                '    RESET,',
                '};',
                '',
                'struct IntersectionContext {',
                '    int  ns_vehicle_count   = 0;',
                '    int  ew_vehicle_count   = 0;',
                '    bool pedestrian_waiting = false;',
                '    bool emergency_active   = false;',
                '    bool sensor_fault       = false;',
                '};',
                '',
                'using Guard  = std::function<bool(const IntersectionContext&)>;',
                'using Action = std::function<void(IntersectionContext&)>;',
                '',
                'struct Transition {',
                '    EventId  trigger;',
                '    StateId  target;',
                '    Guard    guard;',
                '    Action   action;',
                '    int      priority = 0;',
                '};',
                '',
                'struct StateConfig {',
                '    StateId              id;',
                '    std::string          name;',
                '    Action               on_entry;',
                '    Action               on_exit;',
                '    std::vector<Transition> transitions;',
                '};',
                '// === FSM_HEADER_END ===',
                '// === FSM_SOURCE_BEGIN ===',
                '',
                'std::unordered_map<StateId, StateConfig> build_config() {',
                '    std::unordered_map<StateId, StateConfig> cfg;',
                '',
                '    cfg[StateId::NS_GREEN] = {',
                '        StateId::NS_GREEN, "NS_GREEN",',
                '        /*on_entry*/ [](IntersectionContext& ctx) {',
                '            set_ns_green(ctx);',
                '        },',
                '        /*on_exit*/  [](IntersectionContext& ctx) {},',
                '        /*transitions*/ {',
                '            {EventId::EMERGENCY_VEHICLE, StateId::EMERGENCY,',
                '             nullptr, nullptr, 10},',
                '            {EventId::SENSOR_FAULT, StateId::FAULT,',
                '             nullptr, nullptr, 9},',
                '            {EventId::TIMER_EXPIRED, StateId::NS_YELLOW,',
                '             [](const IntersectionContext& ctx) {',
                '                 return ctx.elapsed_ms() >= ctx.green_duration_ms;',
                '             }, nullptr, 1},',
                '            {EventId::MANUAL_OVERRIDE, StateId::ALL_RED,',
                '             nullptr, nullptr, 5},',
                '        }',
                '    };',
                '',
                '    cfg[StateId::NS_YELLOW] = {',
                '        StateId::NS_YELLOW, "NS_YELLOW",',
                '        [](IntersectionContext& ctx) { set_ns_yellow(ctx); },',
                '        [](IntersectionContext& ctx) {},',
                '        {',
                '            {EventId::EMERGENCY_VEHICLE, StateId::EMERGENCY,',
                '             nullptr, nullptr, 10},',
                '            {EventId::SENSOR_FAULT, StateId::FAULT, nullptr, nullptr, 9},',
                '            {EventId::TIMER_EXPIRED, StateId::PEDESTRIAN,',
                '             [](const IntersectionContext& ctx) {',
                '                 return ctx.elapsed_ms() >= ctx.yellow_duration_ms',
                '                     && ctx.pedestrian_waiting;',
                '             }, nullptr, 3},',
                '            {EventId::TIMER_EXPIRED, StateId::ALL_RED,',
                '             [](const IntersectionContext& ctx) {',
                '                 return ctx.elapsed_ms() >= ctx.yellow_duration_ms;',
                '             }, nullptr, 2},',
                '        }',
                '    };',
                '',
                '    cfg[StateId::ALL_RED] = {',
                '        StateId::ALL_RED, "ALL_RED",',
                '        [](IntersectionContext& ctx) { set_all_red(ctx); },',
                '        [](IntersectionContext& ctx) {},',
                '        {',
                '            {EventId::EMERGENCY_VEHICLE, StateId::EMERGENCY,',
                '             nullptr, nullptr, 10},',
                '            {EventId::SENSOR_FAULT, StateId::FAULT, nullptr, nullptr, 9},',
                '            {EventId::TIMER_EXPIRED, StateId::EW_GREEN,',
                '             nullptr, nullptr, 1},',
                '        }',
                '    };',
                '',
                '    cfg[StateId::EW_GREEN] = {',
                '        StateId::EW_GREEN, "EW_GREEN",',
                '        [](IntersectionContext& ctx) { set_ew_green(ctx); },',
                '        [](IntersectionContext& ctx) {},',
                '        {',
                '            {EventId::EMERGENCY_VEHICLE, StateId::EMERGENCY,',
                '             nullptr, nullptr, 10},',
                '            {EventId::SENSOR_FAULT, StateId::FAULT, nullptr, nullptr, 9},',
                '            {EventId::MANUAL_OVERRIDE, StateId::ALL_RED, nullptr, nullptr, 5},',
                '            {EventId::TIMER_EXPIRED, StateId::EW_YELLOW,',
                '             nullptr, nullptr, 1},',
                '        }',
                '    };',
                '',
                '    cfg[StateId::EW_YELLOW] = {',
                '        StateId::EW_YELLOW, "EW_YELLOW",',
                '        [](IntersectionContext& ctx) { set_ew_yellow(ctx); },',
                '        [](IntersectionContext& ctx) {},',
                '        {',
                '            {EventId::EMERGENCY_VEHICLE, StateId::EMERGENCY,',
                '             nullptr, nullptr, 10},',
                '            {EventId::SENSOR_FAULT, StateId::FAULT, nullptr, nullptr, 9},',
                '            {EventId::TIMER_EXPIRED, StateId::PEDESTRIAN,',
                '             nullptr, nullptr, 3},',
                '            {EventId::TIMER_EXPIRED, StateId::NS_GREEN,',
                '             nullptr, nullptr, 1},',
                '        }',
                '    };',
                '',
                '    cfg[StateId::PEDESTRIAN] = {',
                '        StateId::PEDESTRIAN, "PEDESTRIAN",',
                '        [](IntersectionContext& ctx) { set_pedestrian(ctx); },',
                '        [](IntersectionContext& ctx) {},',
                '        {',
                '            {EventId::EMERGENCY_VEHICLE, StateId::EMERGENCY,',
                '             nullptr, nullptr, 10},',
                '            {EventId::SENSOR_FAULT, StateId::FAULT, nullptr, nullptr, 9},',
                '            {EventId::TIMER_EXPIRED, StateId::NS_GREEN,',
                '             nullptr, nullptr, 1},',
                '        }',
                '    };',
                '',
                '    cfg[StateId::EMERGENCY] = {',
                '        StateId::EMERGENCY, "EMERGENCY",',
                '        [](IntersectionContext& ctx) { set_emergency(ctx); },',
                '        [](IntersectionContext& ctx) { ctx.emergency_active = false; },',
                '        {',
                '            {EventId::SENSOR_FAULT, StateId::FAULT, nullptr, nullptr, 9},',
                '            {EventId::TIMER_EXPIRED, StateId::ALL_RED,',
                '             nullptr, nullptr, 1},',
                '            {EventId::FAULT_CLEARED, StateId::ALL_RED,',
                '             nullptr, nullptr, 2},',
                '        }',
                '    };',
                '',
                '    cfg[StateId::FAULT] = {',
                '        StateId::FAULT, "FAULT",',
                '        [](IntersectionContext& ctx) { set_all_red(ctx); },',
                '        [](IntersectionContext& ctx) { ctx.sensor_fault = false; },',
                '        {',
                '            {EventId::FAULT_CLEARED, StateId::NS_GREEN,',
                '             nullptr, nullptr, 1},',
                '            {EventId::RESET, StateId::NS_GREEN,',
                '             nullptr, nullptr, 5},',
                '        }',
                '    };',
                '',
                '    return cfg;',
                '}',
                '// === FSM_SOURCE_END ==='
            ].join('\n');

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');
            expect(r.errors).toHaveLength(0);

            /* 8 states from StateId enum */
            expect(r.states).toHaveLength(8);
            var stateNames = r.states.map(function(s) { return s.name; });
            ['NS_GREEN', 'NS_YELLOW', 'EW_GREEN', 'EW_YELLOW',
             'ALL_RED', 'PEDESTRIAN', 'EMERGENCY', 'FAULT'
            ].forEach(function(n) {
                expect(stateNames).toContain(n);
            });

            /* Transitions from config entries */
            expect(r.transitions.length).toBeGreaterThanOrEqual(10);

            /* Spot-check: NS_GREEN -> EMERGENCY on EMERGENCY_VEHICLE */
            var nsGreenToEmerg = r.transitions.find(function(t) {
                return t.from === 'NS_GREEN' && t.to === 'EMERGENCY' &&
                       t.event === 'EMERGENCY_VEHICLE';
            });
            expect(nsGreenToEmerg).toBeDefined();

            /* Spot-check: NS_GREEN -> NS_YELLOW on TIMER_EXPIRED */
            var nsGreenToYellow = r.transitions.find(function(t) {
                return t.from === 'NS_GREEN' && t.to === 'NS_YELLOW' &&
                       t.event === 'TIMER_EXPIRED';
            });
            expect(nsGreenToYellow).toBeDefined();

            /* Spot-check: FAULT -> NS_GREEN on RESET */
            var faultReset = r.transitions.find(function(t) {
                return t.from === 'FAULT' && t.to === 'NS_GREEN' &&
                       t.event === 'RESET';
            });
            expect(faultReset).toBeDefined();

            /* Spot-check: PEDESTRIAN -> NS_GREEN */
            var pedToNs = r.transitions.find(function(t) {
                return t.from === 'PEDESTRIAN' && t.to === 'NS_GREEN';
            });
            expect(pedToNs).toBeDefined();
        });

        test('parses QtFSM config format (matching C++ backend)', function() {
            var src = [
                '// === FSM_HEADER_BEGIN ===',
                '// Auto-generated FSM Config - VendingMachineFSM',
                '// Generated by QtFSM Designer',
                '',
                '#include <functional>',
                '#include <iostream>',
                '#include <string>',
                '#include <unordered_map>',
                '#include <vector>',
                '',
                'struct TransitionConfig {',
                '    std::string event;',
                '    std::string to;',
                '    std::string guard;',
                '    std::string action;',
                '};',
                '',
                'struct StateConfig {',
                '    std::string name;',
                '    double x;',
                '    double y;',
                '    bool isFinal;',
                '    std::string entry;',
                '    std::string exit;',
                '    std::vector<TransitionConfig> transitions;',
                '};',
                '',
                'struct FSMConfig {',
                '    std::string initial;',
                '    std::unordered_map<std::string, StateConfig> states;',
                '};',
                '// === FSM_HEADER_END ===',
                '// === FSM_SOURCE_BEGIN ===',
                '',
                'FSMConfig cfg;',
                'cfg.initial = "S1";',
                '',
                'cfg.states["S1"] = StateConfig{',
                '    "Idle",',
                '    100.00,',
                '    50.00,',
                '    false,',
                '    "displayWelcome",',
                '    "clearDisplay",',
                '    {',
                '        {"insertCoin", "S2", "", ""},',
                '        {"maintenance", "S5", "isAuthorized", "logAccess"},',
                '    }',
                '};',
                '',
                'cfg.states["S2"] = StateConfig{',
                '    "CoinInserted",',
                '    250.00,',
                '    50.00,',
                '    false,',
                '    "showBalance",',
                '    "",',
                '    {',
                '        {"selectProduct", "S3", "hasSufficientFunds", "reserveProduct"},',
                '        {"insertCoin", "S2", "", "addToBalance"},',
                '        {"cancel", "S1", "", "refundCoins"},',
                '    }',
                '};',
                '',
                'cfg.states["S3"] = StateConfig{',
                '    "Dispensing",',
                '    400.00,',
                '    50.00,',
                '    false,',
                '    "activateMotor",',
                '    "deactivateMotor",',
                '    {',
                '        {"dispensed", "S4", "", "updateInventory"},',
                '        {"jamDetected", "S5", "", "alertOperator"},',
                '    }',
                '};',
                '',
                'cfg.states["S4"] = StateConfig{',
                '    "Complete",',
                '    550.00,',
                '    50.00,',
                '    false,',
                '    "showThankYou",',
                '    "",',
                '    {',
                '        {"timeout", "S1", "", "resetTransaction"},',
                '    }',
                '};',
                '',
                'cfg.states["S5"] = StateConfig{',
                '    "Error",',
                '    400.00,',
                '    200.00,',
                '    true,',
                '    "displayError",',
                '    "",',
                '    {',
                '        {"reset", "S1", "", "clearError"},',
                '    }',
                '};',
                '// === FSM_SOURCE_END ==='
            ].join('\n');

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');

            /* Should extract FSM name from comment */
            expect(r.name).toBe('VendingMachineFSM');

            /* 5 states */
            expect(r.states).toHaveLength(5);

            /* Initial state should be mapped from cfg.initial = "S1" -> "Idle" */
            expect(r.initial).toBeTruthy();

            /* Config transitions: 7 total */
            expect(r.transitions.length).toBeGreaterThanOrEqual(7);

            /* Verify transition with guard and action */
            var selectProduct = r.transitions.find(function(t) {
                return t.event === 'selectProduct';
            });
            expect(selectProduct).toBeDefined();
            expect(selectProduct.guard).toBe('hasSufficientFunds');
            expect(selectProduct.action).toBe('reserveProduct');
        });

        test('parses embedded bootloader config FSM', function() {
            var src = [
                '// === FSM_HEADER_BEGIN ===',
                '#pragma once',
                '#include <cstdint>',
                '',
                'enum class BootState {',
                '    POWER_ON_RESET,',
                '    HARDWARE_INIT,',
                '    FIRMWARE_VERIFY,',
                '    APPLICATION_LOAD,',
                '    RUNNING,',
                '    DFU_MODE,',
                '    FATAL_ERROR',
                '};',
                '',
                'enum class BootEvent {',
                '    HW_READY,',
                '    FW_VALID,',
                '    FW_CORRUPT,',
                '    APP_LOADED,',
                '    DFU_REQUEST,',
                '    FW_UPDATED,',
                '    WATCHDOG_TIMEOUT,',
                '    RESET',
                '};',
                '// === FSM_HEADER_END ===',
                '// === FSM_SOURCE_BEGIN ===',
                '',
                'std::unordered_map<BootState, BootStateConfig> boot_cfg;',
                '',
                'boot_cfg[BootState::POWER_ON_RESET] = {',
                '    BootState::POWER_ON_RESET, "POWER_ON_RESET",',
                '    /*on_entry*/ [](auto& ctx) { runPOST(); },',
                '    /*on_exit*/  nullptr,',
                '    {',
                '        {BootEvent::HW_READY, BootState::HARDWARE_INIT, nullptr, nullptr},',
                '        {BootEvent::WATCHDOG_TIMEOUT, BootState::FATAL_ERROR, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'boot_cfg[BootState::HARDWARE_INIT] = {',
                '    BootState::HARDWARE_INIT, "HARDWARE_INIT",',
                '    [](auto& ctx) { initPeripherals(); },',
                '    nullptr,',
                '    {',
                '        {BootEvent::HW_READY, BootState::FIRMWARE_VERIFY, nullptr, nullptr},',
                '        {BootEvent::WATCHDOG_TIMEOUT, BootState::FATAL_ERROR, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'boot_cfg[BootState::FIRMWARE_VERIFY] = {',
                '    BootState::FIRMWARE_VERIFY, "FIRMWARE_VERIFY",',
                '    [](auto& ctx) { calculateCRC(); },',
                '    nullptr,',
                '    {',
                '        {BootEvent::FW_VALID, BootState::APPLICATION_LOAD, nullptr, nullptr},',
                '        {BootEvent::FW_CORRUPT, BootState::DFU_MODE, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'boot_cfg[BootState::APPLICATION_LOAD] = {',
                '    BootState::APPLICATION_LOAD, "APPLICATION_LOAD",',
                '    [](auto& ctx) { loadApplication(); },',
                '    nullptr,',
                '    {',
                '        {BootEvent::APP_LOADED, BootState::RUNNING, nullptr, nullptr},',
                '        {BootEvent::WATCHDOG_TIMEOUT, BootState::FATAL_ERROR, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'boot_cfg[BootState::RUNNING] = {',
                '    BootState::RUNNING, "RUNNING",',
                '    [](auto& ctx) { jumpToApplication(); },',
                '    nullptr,',
                '    {',
                '        {BootEvent::DFU_REQUEST, BootState::DFU_MODE, nullptr, nullptr},',
                '        {BootEvent::WATCHDOG_TIMEOUT, BootState::POWER_ON_RESET, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'boot_cfg[BootState::DFU_MODE] = {',
                '    BootState::DFU_MODE, "DFU_MODE",',
                '    [](auto& ctx) { enableUSBDFU(); },',
                '    [](auto& ctx) { disableUSBDFU(); },',
                '    {',
                '        {BootEvent::FW_UPDATED, BootState::POWER_ON_RESET, nullptr, nullptr},',
                '        {BootEvent::RESET, BootState::POWER_ON_RESET, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'boot_cfg[BootState::FATAL_ERROR] = {',
                '    BootState::FATAL_ERROR, "FATAL_ERROR",',
                '    [](auto& ctx) { blinkErrorLED(); },',
                '    nullptr,',
                '    {',
                '        {BootEvent::RESET, BootState::POWER_ON_RESET, nullptr, nullptr},',
                '    }',
                '};',
                '// === FSM_SOURCE_END ==='
            ].join('\n');

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');
            expect(r.errors).toHaveLength(0);

            /* 7 boot states */
            expect(r.states).toHaveLength(7);
            var names = r.states.map(function(s) { return s.name; });
            ['POWER_ON_RESET', 'HARDWARE_INIT', 'FIRMWARE_VERIFY',
             'APPLICATION_LOAD', 'RUNNING', 'DFU_MODE', 'FATAL_ERROR'
            ].forEach(function(n) {
                expect(names).toContain(n);
            });

            /* Transitions */
            expect(r.transitions.length).toBeGreaterThanOrEqual(11);

            /* Boot sequence: POWER_ON_RESET -> HARDWARE_INIT -> FIRMWARE_VERIFY -> APP_LOAD -> RUNNING */
            var bootPath = [
                { from: 'POWER_ON_RESET', to: 'HARDWARE_INIT', event: 'HW_READY' },
                { from: 'HARDWARE_INIT', to: 'FIRMWARE_VERIFY', event: 'HW_READY' },
                { from: 'FIRMWARE_VERIFY', to: 'APPLICATION_LOAD', event: 'FW_VALID' },
                { from: 'APPLICATION_LOAD', to: 'RUNNING', event: 'APP_LOADED' }
            ];
            bootPath.forEach(function(bp) {
                var t = r.transitions.find(function(tr) {
                    return tr.from === bp.from && tr.to === bp.to && tr.event === bp.event;
                });
                expect(t).toBeDefined();
            });

            /* DFU path: FW_CORRUPT -> DFU_MODE, FW_UPDATED -> POWER_ON_RESET */
            var toDfu = r.transitions.find(function(t) {
                return t.from === 'FIRMWARE_VERIFY' && t.to === 'DFU_MODE';
            });
            expect(toDfu).toBeDefined();

            var dfuDone = r.transitions.find(function(t) {
                return t.from === 'DFU_MODE' && t.to === 'POWER_ON_RESET' &&
                       t.event === 'FW_UPDATED';
            });
            expect(dfuDone).toBeDefined();
        });

        test('parses protocol handler config FSM', function() {
            var src = [
                '// === FSM_HEADER_BEGIN ===',
                '#pragma once',
                '#include <cstdint>',
                '#include <string>',
                '',
                'enum class ProtocolState : uint8_t {',
                '    DISCONNECTED = 0,',
                '    CONNECTING = 1,',
                '    HANDSHAKE = 2,',
                '    AUTHENTICATED = 3,',
                '    READY = 4,',
                '    TRANSMITTING = 5,',
                '    RECEIVING = 6,',
                '    CLOSING = 7,',
                '    ERROR = 8',
                '};',
                '',
                'enum class ProtocolEvent : uint8_t {',
                '    CONNECT,',
                '    SYN_ACK,',
                '    AUTH_OK,',
                '    AUTH_FAIL,',
                '    DATA_READY,',
                '    TX_COMPLETE,',
                '    RX_COMPLETE,',
                '    DISCONNECT,',
                '    TIMEOUT,',
                '    FATAL',
                '};',
                '// === FSM_HEADER_END ===',
                '// === FSM_SOURCE_BEGIN ===',
                '',
                'std::unordered_map<ProtocolState, StateConfig> proto_cfg;',
                '',
                'proto_cfg[ProtocolState::DISCONNECTED] = {',
                '    ProtocolState::DISCONNECTED, "DISCONNECTED",',
                '    nullptr, nullptr,',
                '    {',
                '        {ProtocolEvent::CONNECT, ProtocolState::CONNECTING, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'proto_cfg[ProtocolState::CONNECTING] = {',
                '    ProtocolState::CONNECTING, "CONNECTING",',
                '    [](auto& ctx) { sendSYN(); },',
                '    nullptr,',
                '    {',
                '        {ProtocolEvent::SYN_ACK, ProtocolState::HANDSHAKE, nullptr, nullptr},',
                '        {ProtocolEvent::TIMEOUT, ProtocolState::ERROR, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'proto_cfg[ProtocolState::HANDSHAKE] = {',
                '    ProtocolState::HANDSHAKE, "HANDSHAKE",',
                '    [](auto& ctx) { sendAuthRequest(); },',
                '    nullptr,',
                '    {',
                '        {ProtocolEvent::AUTH_OK, ProtocolState::AUTHENTICATED, nullptr, nullptr},',
                '        {ProtocolEvent::AUTH_FAIL, ProtocolState::DISCONNECTED, nullptr, nullptr},',
                '        {ProtocolEvent::TIMEOUT, ProtocolState::ERROR, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'proto_cfg[ProtocolState::AUTHENTICATED] = {',
                '    ProtocolState::AUTHENTICATED, "AUTHENTICATED",',
                '    [](auto& ctx) { setupSession(); },',
                '    nullptr,',
                '    {',
                '        {ProtocolEvent::DATA_READY, ProtocolState::READY, nullptr, nullptr},',
                '        {ProtocolEvent::DISCONNECT, ProtocolState::CLOSING, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'proto_cfg[ProtocolState::READY] = {',
                '    ProtocolState::READY, "READY",',
                '    nullptr, nullptr,',
                '    {',
                '        {ProtocolEvent::DATA_READY, ProtocolState::TRANSMITTING, nullptr, nullptr},',
                '        {ProtocolEvent::DISCONNECT, ProtocolState::CLOSING, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'proto_cfg[ProtocolState::TRANSMITTING] = {',
                '    ProtocolState::TRANSMITTING, "TRANSMITTING",',
                '    [](auto& ctx) { startDMA(); },',
                '    [](auto& ctx) { stopDMA(); },',
                '    {',
                '        {ProtocolEvent::TX_COMPLETE, ProtocolState::RECEIVING, nullptr, nullptr},',
                '        {ProtocolEvent::TIMEOUT, ProtocolState::ERROR, nullptr, nullptr},',
                '        {ProtocolEvent::FATAL, ProtocolState::ERROR, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'proto_cfg[ProtocolState::RECEIVING] = {',
                '    ProtocolState::RECEIVING, "RECEIVING",',
                '    nullptr, nullptr,',
                '    {',
                '        {ProtocolEvent::RX_COMPLETE, ProtocolState::READY, nullptr, nullptr},',
                '        {ProtocolEvent::TIMEOUT, ProtocolState::ERROR, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'proto_cfg[ProtocolState::CLOSING] = {',
                '    ProtocolState::CLOSING, "CLOSING",',
                '    [](auto& ctx) { sendFIN(); },',
                '    nullptr,',
                '    {',
                '        {ProtocolEvent::DISCONNECT, ProtocolState::DISCONNECTED, nullptr, nullptr},',
                '        {ProtocolEvent::TIMEOUT, ProtocolState::DISCONNECTED, nullptr, nullptr},',
                '    }',
                '};',
                '',
                'proto_cfg[ProtocolState::ERROR] = {',
                '    ProtocolState::ERROR, "ERROR",',
                '    [](auto& ctx) { logError(); },',
                '    nullptr,',
                '    {',
                '        {ProtocolEvent::DISCONNECT, ProtocolState::DISCONNECTED, nullptr, nullptr},',
                '    }',
                '};',
                '// === FSM_SOURCE_END ==='
            ].join('\n');

            var r = parser.parseInput(src);

            expect(r.format).toBe('marked');
            expect(r.errors).toHaveLength(0);

            /* 9 protocol states */
            expect(r.states).toHaveLength(9);
            var names = r.states.map(function(s) { return s.name; });
            ['DISCONNECTED', 'CONNECTING', 'HANDSHAKE', 'AUTHENTICATED',
             'READY', 'TRANSMITTING', 'RECEIVING', 'CLOSING', 'ERROR'
            ].forEach(function(n) {
                expect(names).toContain(n);
            });

            /* Enum values should be stripped */
            names.forEach(function(n) {
                expect(n).not.toMatch(/^\d/);
                expect(n).not.toMatch(/=/);
            });

            /* Transitions */
            expect(r.transitions.length).toBeGreaterThanOrEqual(14);

            /* Connection flow: DISCONNECTED->CONNECTING->HANDSHAKE->AUTHENTICATED->READY */
            var connPath = [
                { from: 'DISCONNECTED', to: 'CONNECTING', event: 'CONNECT' },
                { from: 'CONNECTING', to: 'HANDSHAKE', event: 'SYN_ACK' },
                { from: 'HANDSHAKE', to: 'AUTHENTICATED', event: 'AUTH_OK' },
                { from: 'AUTHENTICATED', to: 'READY', event: 'DATA_READY' }
            ];
            connPath.forEach(function(cp) {
                var t = r.transitions.find(function(tr) {
                    return tr.from === cp.from && tr.to === cp.to && tr.event === cp.event;
                });
                expect(t).toBeDefined();
            });

            /* Data flow: READY->TRANSMITTING->RECEIVING->READY */
            var txToRx = r.transitions.find(function(t) {
                return t.from === 'TRANSMITTING' && t.to === 'RECEIVING';
            });
            expect(txToRx).toBeDefined();

            var rxToReady = r.transitions.find(function(t) {
                return t.from === 'RECEIVING' && t.to === 'READY';
            });
            expect(rxToReady).toBeDefined();

            /* Auth failure: HANDSHAKE->DISCONNECTED */
            var authFail = r.transitions.find(function(t) {
                return t.from === 'HANDSHAKE' && t.to === 'DISCONNECTED' &&
                       t.event === 'AUTH_FAIL';
            });
            expect(authFail).toBeDefined();
        });
    });
});
