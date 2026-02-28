#!/bin/bash

# Simple Controller for the BS Solver
# Allows pausing and resuming the solver safely

PID_FILE=".solver_pid"

case "$1" in
    start)
        if [ -f "$PID_FILE" ] && kill -0 $(cat "$PID_FILE") 2>/dev/null; then
            echo "Solver is already running with PID $(cat $PID_FILE)"
        else
            echo "Starting BS(45) Solver in the background..."
            nohup ./bin/wz_sa 44 >> solver.log 2>&1 &
            PID=$!
            echo $PID > "$PID_FILE"
            echo "Solver started with PID $PID. Output is being logged to solver.log"
            echo "Use './manage_solver.sh tail' to view progress."
        fi
        ;;
    pause)
        if [ -f "$PID_FILE" ]; then
            PID=$(cat "$PID_FILE")
            if kill -0 $PID 2>/dev/null; then
                kill -STOP $PID
                echo "Solver (PID $PID) has been PAUSED. It is using 0% CPU but keeping its place in memory."
            else
                echo "Solver is not currently running."
            fi
        else
            echo "No solver PID found."
        fi
        ;;
    resume)
        if [ -f "$PID_FILE" ]; then
            PID=$(cat "$PID_FILE")
            if kill -0 $PID 2>/dev/null; then
                kill -CONT $PID
                echo "Solver (PID $PID) has been RESUMED."
            else
                echo "Solver is not currently running."
            fi
        else
            echo "No solver PID found."
        fi
        ;;
    stop)
        if [ -f "$PID_FILE" ]; then
            PID=$(cat "$PID_FILE")
            if kill -0 $PID 2>/dev/null; then
                kill -TERM $PID
                echo "Solver (PID $PID) has been STOPPED completely."
            else
                echo "Solver is not running."
            fi
            rm "$PID_FILE"
        else
            echo "No solver PID found."
        fi
        ;;
    tail)
        if [ -f "solver.log" ]; then
            tail -f solver.log
        else
            echo "Log file not found."
        fi
        ;;
    status)
        if [ -f "$PID_FILE" ]; then
            PID=$(cat "$PID_FILE")
            if kill -0 $PID 2>/dev/null; then
                STATE=$(ps -o stat= -p $PID 2>/dev/null)
                if [[ "$STATE" == *"T"* ]]; then
                    echo "Solver is currently PAUSED (PID $PID)."
                else
                    echo "Solver is currently RUNNING (PID $PID)."
                fi
            else
                echo "Solver is STOPPED/NOT RUNNING."
            fi
        else
            echo "Solver is STOPPED/NOT RUNNING."
        fi
        ;;
    *)
        echo "Usage: ./manage_solver.sh {start|pause|resume|status|tail|stop}"
        exit 1
        ;;
esac
