#include "CodeParser.h"
#include "../model/FSM.h"
#include "../model/State.h"
#include "../model/Transition.h"
#include "../model/Event.h"
#include <QRegularExpression>
#include <QDebug>

CodeParser::CodeParser()
{
}

QString CodeParser::lastError() const
{
    return m_lastError;
}

FSM* CodeParser::parse(const QString &code, QObject *parent)
{
    if (code.isEmpty()) {
        m_lastError = "Empty code";
        return nullptr;
    }
    
    FSM *fsm = new FSM(parent);
    
    // 1. Try to find FSM name
    // Pattern: class Name : public State/FSM ...
    // For now detailed parsing is limited, so we'll look for simple patterns
    // matching the code we generate
    
    // Parse States
    // Improve regex to be even more flexible. Capture Base class too.
    // "class Name : public BaseClass"
    QRegularExpression stateRegex("class\\s+([A-Za-z0-9_]+)\\s*:\\s*public\\s+([A-Za-z0-9_]+)");
    QRegularExpressionMatchIterator i = stateRegex.globalMatch(code);
    
    QMap<QString, State*> nameToStateMap;
    
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString className = match.captured(1);
        QString baseName = match.captured(2);
        
        // Match UI names by stripping "State" suffix if present
        // Generator adds "State" suffix, so we remove it to match original names
        QString stateName = className;
        if (stateName.endsWith("State") && stateName.length() > 5) {
            stateName.chop(5);
        }
        
        bool isValidBase = baseName == "State" || baseName.contains("State") || baseName.contains("Base");
        
        if (isValidBase && !className.contains("Context")) {
             State *state = new State(stateName, stateName, fsm);
             fsm->addState(state);
             // We map properties to the parsed class name for transition lookup
             nameToStateMap.insert(className, state);
        }
    }
    
    if (fsm->states().isEmpty()) {
        m_lastError = "No states found. Ensure your classes inherit from a State base class (e.g. 'class MyState : public MyStateBase')";
        delete fsm;
        return nullptr;
    }
    
    // Parse Transitions (Basic)
    // We look for patterns like: void react(Event& e) { ... changeState(new TargetState()); ... }
    // OR "return new TargetState()" (State Pattern style)
    
    // 1. Locate start of each class
    QVector<int> classOffsets;
    QVector<QString> classNames;
    
    i = stateRegex.globalMatch(code); // re-run or just store from above
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        classOffsets.append(match.capturedStart());
        classNames.append(match.captured(1));
    }
    classOffsets.append(code.length()); // End sentinel
    
    // 2. Scan inside each block
    for (int k = 0; k < classNames.size(); ++k) {
        QString currentName = classNames[k];
        State *source = nameToStateMap.value(currentName);
        if (!source) continue;
        
        int start = classOffsets[k];
        int end = classOffsets[k+1];
        QString block = code.mid(start, end - start);
        
        // Regex for transition: 
        // 1. changeState(new Target(...))
        // 2. return new Target(...)
        QRegularExpression transRegex("(?:changeState\\s*\\(|return\\s+)(?:new\\s+)([A-Za-z0-9_]+)");
        QRegularExpressionMatchIterator ti = transRegex.globalMatch(block);
        
        while (ti.hasNext()) {
            QRegularExpressionMatch tMatch = ti.next();
            QString targetName = tMatch.captured(1);
            
            // Strip "State" suffix from target name to match our keys
            if (targetName.endsWith("State") && targetName.length() > 5) {
                targetName.chop(5);
            }
            
            // We mapped original full class names in map for property lookup if needed, 
            // BUT we inserted them into nameToStateMap using the *sanitized* (stripped) name?
            // Let's check above: 
            // nameToStateMap.insert(className, state); -> className is "State1State"
            // Wait, we need to be careful.
            // If we changed the key above to stripped name, we must use stripped name here.
            // Let's assume we fixed the key above to be based on "State1State" (className) 
            // OR we used "State1" (stateName).
            
            // Checking previous step: 
            // nameToStateMap.insert(className, state); 
            // -> Key is "State1State". 
            // -> State name is "State1".
            
            // So if we find "State2State", we should look for "State2State".
            // So we do NOT strip it for lookup if the map key is full class name.
            
            // Let's revert the strip for lookup if keys are class names.
            // But wait, earlier I said "My state map keys are State2 (stripped)".
            // Let's verify what I wrote in previous step.
            // "nameToStateMap.insert(className, state);" -> className is the captured group 1 from class regex.
            // "class State1State" -> className = "State1State".
            // So keys ARE "State1State".
            
            // So if code has "return new State2State", we capture "State2State".
            // We lookup "State2State". It should match!
            
            // So stripping is NOT needed for lookup if keys are class names.
            // BUT, if the code uses "return new State2", and class was "State2", it works.
            
            // Let's just lookup targetName directly first.
            State *target = nameToStateMap.value(targetName);
            
            // If not found, maybe try appending or stripping "State"?
            if (!target && !targetName.endsWith("State")) {
                 target = nameToStateMap.value(targetName + "State");
            }
            
            // ... (rest)
            // ... (rest same as before)
            if (target) {
                // Check if transition already exists to avoid duplicates (naive check)
                bool exists = false;
                for (Transition *t : source->transitions()) {
                    if (t->targetState() == target) {
                        exists = true;
                        break;
                    }
                }
                
                if (!exists) {
                    Transition *t = new Transition(source, target);
                    t->setEvent("Event"); // Default or detected event
                    source->addTransition(t);
                }
            }
        }
    }
    
    // Set first found state as initial
    if (!fsm->states().isEmpty()) {
        fsm->setInitialState(fsm->states().first());
        fsm->states().first()->setInitial(true);
    }
    
    return fsm;
}
