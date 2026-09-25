# Your HPC & SLURM Interview Prep Guide

This guide is designed to take you from absolute zero to completely confident. It breaks down everything you have been doing into plain English so you can explain it to any audience.

---

## SECTION 1: WHAT IS GOING ON OVERALL

**What is HPC (High-Performance Computing)?**
Simply put, HPC is using supercomputers to solve massive math or data problems that would crash a normal laptop or take hundreds of years to finish. 

**What is a Cluster?**
A cluster is basically hundreds or thousands of computers stacked in racks in a server room, all connected together by super-fast cables so they can act like one giant computer.

**What is a Node?**
A "node" is just a single computer inside that cluster. Think of a cluster like an apartment building, and a node is a single apartment. Most of the nodes you used had 192 CPUs (cores) living inside that one apartment!

**What is SLURM?**
SLURM is the manager (or the bouncer) of the cluster. When 500 different scientists all want to run their code on the cluster at the same time, it would be chaos. SLURM puts their requests into a line (a queue), looks at what computers (nodes) are currently free, and assigns the work fairly. 

**Why use SLURM instead of a laptop?**
Your laptop has maybe 8 or 10 "cores" (brains). If you try to run a massive search problem on it, it will overheat and take weeks. On a cluster, you can ask SLURM for 3,840 cores at the exact same time. It finishes the work in a fraction of the time.

**What Ross / Cylix likely wants from someone with SLURM experience:**
Ross has clients (or internal teams) who have massive AI or data programs. They need to run these programs on supercomputers. He needs someone who knows how to format the code properly, talk to the SLURM bouncer, get the work scheduled efficiently without wasting money or resources, and figure out what went wrong when a job inevitably fails.

**What "contract work / training" most likely means:**
It probably means he has junior engineers or data scientists who know how to build AI models on their laptops, but have no idea how to put those models onto a supercomputer. Your job would be writing the scripts (instructions) for the supercomputer, solving the errors when they happen, and writing guides or teaching his team how to submit their own jobs.

---

## SECTION 2: THE FULL REAL-WORLD FLOW, STEP BY STEP

Imagine you are mailing a really difficult homework assignment to a team of math geniuses.

**1. I write code on my computer**
You sit at your local laptop (a regular Mac or Windows machine). You write your C++ code.

**2. Then what?**
You need to get this code from your laptop over to the supercomputer. Because the supercomputer is in a different city (or province), you open a window on your computer to talk to it over the internet.

**3. How do I connect to the cluster?**
You use an application called the **Terminal** (a black screen where you type text commands) and a tool called **SSH** (Secure Shell). SSH is basically a secure, invisible tunnel that connects your computer to the supercomputer.

**4. What am I using to connect?**
You are mostly using the Terminal. Once you connect, your terminal stops looking at your laptop's files and starts looking at the supercomputer's files.

**5. Am I uploading a .c file directly?**
Yes. You transfer your raw `code.cpp` file over to the cluster.

**6. Am I uploading a shell script too?**
Yes. You also upload a small text file with instructions for SLURM (this is called an `sbatch script` or `job script`).

**7. Does the cluster compile the C file itself?**
Yes! A cluster has different types of nodes (computers). You connect to a **Login Node**—this is just the lobby of the building. You don't do heavy work here, but you do compile the code here. You type a command to turn your readable text (`.cpp` file) into a **compiled executable** (a machine-code program the computer can actually run). 

**8. What is a shell script / .sh file in this context?**
If the compiled executable is the "machinery", the shell script is the "instruction manual" for the bouncer. It says: *"Hi SLURM, my name is Daniel. Please give me 1 node with 192 cores for exactly 24 hours. When you find a free node, go there and click 'Play' on my compiled executable. If it crashes, email me."*

**9. What does SLURM actually receive from me?**
SLURM receives your job script. You literally hand it to SLURM by typing the command `sbatch my_script.sh`.

**10. What happens after I submit a job?**
SLURM says, "Okay Daniel, here is your receipt (Job ID: 123456). Your job is now in the **Job Queue**." It waits in line until the resources you asked for become available.

**11. Where does the code run?**
When it's your turn, SLURM sneaks your compiled code out the back door of the lobby and puts it onto a **Compute Node** (the actual heavy-lifting computers hidden inside the cluster).

**12. Where do output and errors go?**
Because you aren't sitting at the compute node, any text your program prints (like "Calculating..." or "Oh no, error!") is saved into simple text files—an **Output file** for normal text, and an **Error file** if things break.

**13. How do I know if it worked?**
You look at the text files. You can also type commands into the terminal (like `sq` or `squeue`) to ask SLURM, "Hey, what is the status of Job 123456?"

**14. How do I download or inspect results afterward?**
You can read the result files right there in the terminal on the login node. If it's a massive data spreadsheet, you use another text-tunnel tool (like `scp` or `sftp`) to pull the file back down to your personal laptop.

---

## SECTION 3: MAKE IT VISUAL IN WORDS

**My laptop** -> **SSH** -> **cluster login node** -> **compile code** -> **create sbatch script** -> **submit with sbatch** -> **SLURM queue** -> **compute node runs job** -> **output files**

*Translation:*
1. **My laptop**: Where I sit and type locally.
2. **SSH**: The secure invisible tunnel over the internet.
3. **cluster login node**: The "lobby" of the supercomputer where I first land and organize my files.
4. **compile code**: Translating my human-readable C++ code into machine code on the login node.
5. **create sbatch script**: Writing the instruction manual for the bouncer.
6. **submit with sbatch**: Handing the instruction manual to the SLURM bouncer.
7. **SLURM queue**: My job waits in line while other people finish theirs.
8. **compute node runs job**: The bouncer gives me a room, and my code runs on 192 massive brains at once.
9. **output files**: When it finishes, it leaves a sticky note on my desk (the output text file) so I can see the result.

---

## SECTION 4: EXPLAIN MY OWN WORK IN SIMPLE TERMS

**A. Technical Explanation**
"In my research at Laurier, I managed large-scale parallel search workloads utilizing Compute Canada's national supercomputing clusters, specifically Rorqual and Trillium. I wrote parallelized C++ algorithms using OpenMP, and deployed them across the cluster using SLURM job arrays. On average, I coordinated distributed execution across dozens of nodes simultaneously—effectively managing tens of thousands of CPU cores. I wrote the job scripts, requested optimal CPU and memory allocations, managed the job queues, and built pipelines to analyze the stderr and stdout logs to verify successful executions."

**B. Non-Technical Explanation**
"I helped my university solve extremely complicated math problems. Standard computers couldn’t process the data fast enough, so I used the Canadian national supercomputers. My job was writing the instruction scripts that allowed our code to split up its work and run across thousands of different computer brains simultaneously. I handled the scheduling, made sure we were using the computers efficiently, and monitored the outputs to ensure our jobs didn't crash before finding the answers we needed."

**C. One-minute explanation for the call**
"In my research role, I basically acted as the bridge between complex math code and the national supercomputing infrastructure. We were running massive parallel search algorithms. My job was to take that code, get it onto the clusters, and use SLURM to direct traffic. I wrote the shell scripts to deploy our workloads using SLURM job arrays, which allowed me to spin up 20 to 50 nodes at once—meaning I was orchestrating about 3,800 to 10,000 CPU cores simultaneously. I managed the resource requests, monitored the queues, and handled the debugging when things failed. It gave me a very strong, practical foundation in exactly how SLURM works in a large production environment."

**D. 20-second emergency explanation** (If nervous)
"I ran massive parallel computations for a research project on Canada's national supercomputers. I wrote the SLURM scripts to distribute our code across thousands of CPU cores simultaneously, managed the job queues, and debugged the workloads when they crashed. I'm very comfortable getting code off a laptop and running it efficiently on a massive cluster."

---

## SECTION 5: WHAT I PROBABLY ACTUALLY DID

*Based on looking at your actual code and scripts (like `rorqual_bs43_job.sh` and `bs45_job.sh`), here is exactly what your workflow was.*

* **Was I probably using SSH to connect?** YES. You used SSH to connect to clusters like `rorqual` and `trillium` (Compute Canada).
* **Was I probably editing code locally or remotely?** You probably coded locally in VS Code or similar, then pushed to GitHub or used `scp` to move files over, though you also tweaked scripts on the cluster.
* **Was I probably compiling on the cluster?** YES. I can see in your scripts it literally says checking for `wz_sa` and if not found, it runs `g++ -O3 -march=native ...`.
* **Was I probably using bash / shell scripts?** YES. Your `.sh` files are standard bash scripts.
* **Was I probably using SLURM job arrays?** YES. Your Rorqual script uses `#SBATCH --array=0-19`. You used this to launch 20 identical jobs with slightly different math starting points (seed offsets) simultaneously.
* **Was I probably dealing with stdout/stderr logs?** YES. You specified `#SBATCH --output=bs43_rorqual_output_%A_%a.txt` which means your results and errors printed right to text files.
* **Was I probably requesting CPUs, memory, and wall-clock time?** YES. I can see you requested exactly 24:00:00 time, and 192 cores per task. 

*If Ross asks how you did it, you used OpenMP to let your C++ code use all 192 cores locally on a single node, and you used SLURM Job Arrays to launch 20 of those nodes at the same time.*

---

## SECTION 6: TEACH ME THE COMMANDS LIKE I’M NEW

**ssh**
* **What it does:** Creates a secure connection from your laptop to the cluster.
* **Why use it:** To log into the supercomputer so you can type commands there.
* **Example:** `ssh daniel@trillium.computecanada.ca`
* **Plain English:** Ringing the doorbell of the supercomputer and showing your ID badge.

**ls** (List)
* **What it does:** Lists all the files and folders in your current directory.
* **Why use it:** To see if your uploaded file actually arrived.
* **Example:** `ls`
* **Plain English:** Opening a filing cabinet and looking at exactly what folders are inside.

**cd** (Change Directory)
* **What it does:** Moves you into a different folder.
* **Why use it:** To navigate around your files.
* **Example:** `cd projects/physics`
* **Plain English:** Walking into a different room.

**pwd** (Print Working Directory)
* **What it does:** Tells you exactly what folder you are currently inside.
* **Why use it:** Because the terminal has no graphical interface, it's easy to get lost.
* **Example:** `pwd` (Returns `/home/daniel/projects`)
* **Plain English:** Looking at the "You Are Here" dot on a mall map.

**nano / vim / code**
* **What it does:** Text editors inside the terminal.
* **Why use it:** To quickly change a line of code or a script without re-uploading the file perfectly.
* **Example:** `nano my_script.sh`
* **Plain English:** Opening up Microsoft Word inside the black text screen.

**gcc / g++**
* **What it does:** These are C and C++ compilers.
* **Why use it:** The computer doesn't understand english text. `g++` translates it to math/machine code.
* **Example:** `g++ start.cpp -o my_program`
* **Plain English:** Translating a book from English into a language the computer can read.

**chmod +x**
* **What it does:** Changes file permissions to make a file "eXecutable".
* **Why use it:** Sometimes the cluster refuses to run a file for safety reasons until you mark it as safe.
* **Example:** `chmod +x my_program`
* **Plain English:** Giving yourself VIP permission to push the "Go" button on a machine.

**./program**
* **What it does:** Runs an executable file that is in your current folder.
* **Why use it:** To test if the code actually works before giving it to SLURM.
* **Example:** `./wz_sa`
* **Plain English:** Hitting the "Play" button on your app.

**sbatch**
* **What it does:** Submits your job to SLURM.
* **Why use it:** This is THE primary command of SLURM. It enters you into the queue.
* **Example:** `sbatch bs45_job.sh`
* **Plain English:** Handing a pizza order sheet to the chef.

**squeue**
* **What it does:** Checks the status of jobs in the cluster.
* **Why use it:** To see if your job is running, pending, or finished.
* **Example:** `squeue -u daniel` (shows only your jobs)
* **Plain English:** Looking at the digital TV screen at McDonald's to see if your order number is ready.

**scancel**
* **What it does:** Kills a job.
* **Why use it:** You realized you made a typo in the code and want to stop you job before you ruin your compute budget.
* **Example:** `scancel 123456`
* **Plain English:** Yelling "Stop!" to the chef and ripping up the order.

**sacct**
* **What it does:** Shows accounting data for old jobs.
* **Why use it:** To look at a job that finished yesterday and see how much memory it actually used.
* **Example:** `sacct -j 123456`
* **Plain English:** Looking at the receipt after the meal.

**cat / less / tail**
* **What it does:** Prints the text inside a file right onto the screen.
* **Why use it:** To quickly read an output or error log without fully opening it in an editor.
* **Example:** `tail error.txt` (Shows the last 10 lines, useful to see how a program crashed).
* **Plain English:** Quickly glancing at the bottom of a document.

---

## SECTION 7: SBATCH / SHELL SCRIPT EXPLAINED VERY SIMPLY

**What is a shell script?**
A regular text file filled with terminal commands. Instead of you typing `load this, compile that, run this` one by one, the script automatically types them for you.

**What is an sbatch script?**
It is just a shell script, but with a special VIP section at the very top. The VIP section contains instructions exclusively for SLURM. Every SLURM instruction starts with `#SBATCH`.

**What these lines mean:**
* `#!/bin/bash` -> The first line of every script. It just tells the computer "read this file as a bash shell script".
* `#SBATCH --job-name=test` -> Labels your job "test" so you can recognize it in the queue.
* `#SBATCH --output=out.txt` -> Tells SLURM "save any normal printed text into a file called out.txt".
* `#SBATCH --error=err.txt` -> Tells SLURM "save any error messages or crash reports into err.txt".
* `#SBATCH --time=01:00:00` -> "I promise my job will take no more than 1 hour. If it hits 1 hour and 1 second, kill it."
* `#SBATCH --mem=4G` -> "I need 4 gigabytes of RAM. If I use 4.1GB, crash the code."
* `#SBATCH --cpus-per-task=4` -> "Give me 4 brains dedicated to my math."

**A tiny workflow example:**
Let's say you upload a math program `math.cpp` and a script `job.sh`.
1. **Compile it:** You type `g++ math.cpp -o math_exec`.
2. **Submit it:** You type `sbatch job.sh`.
3. **How SLURM runs it:** SLURM reads the `#SBATCH` tags, finds a node with enough space, securely brings your `math_exec` over there, and runs it blindly. It takes whatever `math_exec` spits out and dumps it in `out.txt`.

---

## SECTION 8: JOB ARRAYS AND PARALLEL WORKLOADS

*This is your superpower. Very few beginners know this.*

**What "parallel search workloads" means:**
Instead of having 1 person search a whole warehouse for a lost keychain, you hire 192 people to search the warehouse at the same time. You were running code that used 192 brains simultaneously.

**What a Job Array is:**
Imagine you need to run the exact same program 20 times, but just changing a single number (the starting coordinate) each time. Instead of typing `sbatch` 20 times and making 20 scripts, you create ONE script and add `#SBATCH --array=0-19`. SLURM instantly duplicates it into 20 perfectly managed jobs labeled 0, 1, 2, etc. 

**What "distributed array jobs" means:**
Because you used an array, SLURM "distributed" or handed out those 20 jobs to 20 completely different computer nodes across the entire cluster.

**What "coordinating tens of thousands of CPU cores" means:**
You asked for 20 jobs (nodes). Each node had 192 CPUs. 20 x 192 = 3,840 cores running at the exact same exact second. If you did this a few times a week, you easily coordinated tens of thousands of processing hours. 

**How to explain this to a non-technical person:**
"Imagine a company needs to render a 2-hour Pixar movie. Doing it on one computer would take 5 years. I know how to write the instructions to take that movie, chop it up into thousands of frames, and send it to thousands of computers at the exact same time. What takes a normal computer five years, I can finish in an hour."

---

## SECTION 9: COMMON PROBLEMS AND SIMPLE DEBUGGING

**Job Pending** (Wait state)
* **What it means:** The cluster is full. You asked for 10 nodes for 7 days, and SLURM can't find that much free space right now.
* **Where to look:** Run `squeue`. The status will say `PD` (Pending).
* **Explain simply:** "We ordered a massive table for 50 people, we just have to wait for the restaurant to clear a spot."

**Wrong Partition**
* **What it means:** You sent your job to the wrong section of the cluster. E.g., You asked the "short jobs lane" (which limits time to 3 hours) for a 24-hour job.
* **Where to look:** You will get an immediate rejection error when you type `sbatch`.
* **Explain simply:** "Like going into the 10-items-or-less checkout lane with a full cart of groceries."

**Out of Memory (OOM)**
* **What it means:** The code tried to store 5GB of data, but your `#SBATCH --mem` only asked for 4GB. SLURM killed it instantly to protect other users.
* **Where to look:** Your `.err` file will literally say `Out Of Memory` or `OOM-Killed`.
* **Explain simply:** "We tried to pour a gallon of water into a shot glass."

**Job Timed Out**
* **What it means:** You asked for 24 hours. The computation was so massive it took 25. At exactly 24:00:00, SLURM murdered your job.
* **Where to look:** The `.out` or `.err` file will abruptly end with a "CANCELLED DUE TO TIME LIMIT" message.
* **Explain simply:** "Our parking meter ran out of time and our car got towed."

**Permission Issue**
* **What it means:** You tried to run a script, but Linux says you don't have the security clearance.
* **Where to look:** Terminal outputs `Permission denied`. Use `chmod +x` to fix.
* **Explain simply:** "The file was locked inside a safe and we didn't use the key."

---

## SECTION 10: WHAT ROSS IS MOST LIKELY TO ASK

**Q: What is your actual experience with SLURM?**
* **Real Meaning:** Have you actually used it in the real world, or just read a tutorial?
* **Simple:** "I used it heavily in my university research to run massive parallel simulations."
* **Technical:** "I spent 4 months deploying OpenMP C++ workloads on national computing clusters (Rorqual, Trillium) using SLURM. I regularly managed job arrays spanning thousands of collective CPU cores to handle search space optimization."

**Q: How do you submit a job?**
* **Real Meaning:** Do you know the exact command and what a script looks like?
* **Simple:** "You write a text file with instructions, and type `sbatch filename.sh`."
* **Technical:** "I create a shell script declaring resources via `#SBATCH` directives, like `--time`, `--nodes`, and `--cpus-per-task`, load all necessary environment modules, and trigger it with `sbatch`."

**Q: How do you debug failures?**
* **Real Meaning:** If a job dies silently, do you panic or do you investigate?
* **Simple:** "I immediately check the error output files that SLURM generates to see what went wrong."
* **Technical:** "I check the `stdout` and `stderr` files first. If it's resource-related (like OOM or timeout), I'll use `sacct` to review job efficiency data to see if I under-requested RAM or wall-clock time."

**Q: What are job arrays?**
* **Real Meaning:** Can you manage large-scale data quickly?
* **Simple:** "It's a way to submit hundreds of identical jobs at once without writing hundreds of scripts."
* **Technical:** "SLURM arrays (`--array=0-100`) let you use a single script to dispatch multiple parallel tasks. You pass the `SLURM_ARRAY_TASK_ID` dynamic variable into the script to offset the data slice or random seed for each node."

**Q: How do you know how much memory or CPU to request?**
* **Real Meaning:** Are you going to waste my company's expensive cloud money by requesting 500GB of RAM for a 2GB task?
* **Simple:** "I run a small, 10-minute test job first. I see what it uses, and then I ask for just a little bit more than that."
* **Technical:** "I profile the code on a small dataset, monitor the usage, pad it by about 20% for safety, and scale the request accordingly."

**Q: What kind of contract work do you think this is?**
* **Real Meaning:** Do you understand what my company needs you to do for me?
* **Simple / Technical:** "It sounds like you need someone who can jump in, bridge the gap between your AI code and your infrastructure, manage the job deployment effectively, and potentially create workflows or documentation so your broader team can submit their SLURM jobs successfully."

---

## SECTION 11: HOW TO TEACH THIS TO SOMEONE ELSE
*Use these exact lines if Ross asks: "How would you explain X to a junior developer?"*

**"What is a cluster?"**
"Imagine you have one incredibly smart employee. That's a laptop. Now imagine you hire 5,000 smart employees and put them in a giant stadium, and they can all talk to each other instantly. That's a cluster. "

**"What is SLURM?"**
"SLURM is the traffic cop. When 50 developers all want to jump into the stadium at once, the whole system would crash. SLURM takes your request, puts you in line, and politely leads you to a block of empty seats when they are ready."

**"How do I run my first job?"**
"It's just two steps. Your code is the 'car', and you need to write a little sticky note (a shell script) that says 'I need gas for 2 hours.' Then you hand that sticky note to the traffic cop using the command `sbatch`."

**"How do I know if it worked?"**
"You’re completely blind while it runs in a different room. But SLURM is great. You just ask SLURM to slide a piece of paper under the door when it's done. Those are your `.out` and `.err` text files."

---

## SECTION 12: WHAT MAKES ME QUALIFIED

**Why you are qualified for this call:**
You have *actual, real-world experience* doing this on highly restricted, professional-grade national computing hardware. Many software engineers go years without ever touching a command line outside their local computer. You have successfully run massive math problems spread out across thousands of CPU cores.

**Your Strengths:**
* You grasp parallel computing (OpenMP).
* You grasp distributed deploying (SLURM arrays, clusters).
* You know how to debug terminal environments.
* You know how to act independently.

**Where you are still junior:**
You have only done this for 4 months natively. You probably don't have experience being the *Administrator* of SLURM (e.g., actually installing SLURM software onto a brand-new metal server). You are an advanced *user* of SLURM. 

**How to talk about that without sounding weak:**
"I wouldn't call myself a DevOps infrastructure architect who installs SLURM from scratch onto bare-metal hardware. My expertise is on the application and execution side: I’m highly comfortable as an advanced end-user taking AI/ML code, scripting the deployment, managing the resources, arraying jobs across the cluster, and debugging."

---

## SECTION 13: FINAL CHEAT SHEET

**15 Key Terms to remember:**
1. **Node** (One computer in the rack)
2. **Cluster** (The whole warehouse of computers)
3. **Core / CPU** (The individual brains inside the node)
4. **SLURM** (The queue manager software)
5. **Job** (A task you give to SLURM)
6. **Queue** (The waiting line)
7. **Partition** (Different queues for different sizes of jobs)
8. **sbatch** (The command to submit a job)
9. **Shell Script** (The instruction text file)
10. **Stdout/Stderr** (The standard output text and error text)
11. **Job Array** (Submitting hundreds of similar jobs instantly)
12. **SSH** (The terminal tunnel to the cluster)
13. **Login Node** (The lobby where you compile, but DO NOT run heavy code)
14. **Compute Node** (The back rooms where heavy code actually runs)
15. **Wall-clock time** (Real-world human time required for the job)

**10 Strongest Phrases to drop in the call:**
1. "Parallelizing workloads."
2. "Managing resource allocation and request limits."
3. "Executing distributed computations."
4. "Job arrays."
5. "Debugging stdout and stderr logs."
6. "Bridging the gap between the code and the infrastructure."
7. "Search-space convergence." (A nod to your simulated annealing work!)
8. "Monitoring queue efficiency."
9. "OpenMP multithreading on single nodes, distributed via SLURM."
10. "I’m very comfortable in a headless Linux terminal environment."

---

## SECTION 14: EXPLAINING THE ACTUAL MATH SIMPLY
*If Ross asks: "So what was the C++ code actually calculating?"*

**The 30-Second Explanation:**
"My C++ code was hunting for incredibly rare mathematical patterns used in making secure digital signals and cryptography. 
Imagine trying to create a secret radio signal that has absolutely zero 'echo' or static interference. To do this, we generate four massive sequences of numbers (we'll call them A, B, C, and D). The C++ program overlaps them, shifts them around millions of times, and adds them together. 
Our goal is to find a very specific, needle-in-a-haystack combination where all the overlaps perfectly cancel each other out to exactly **zero**. 
If we hit zero, we've discovered a special mathematical structure called a **Hadamard Matrix**. In the real world, these 'perfect zero-echo' codes are what engineers use to build tamper-proof cryptography, perfect wireless networks, and military radar that never shows a false positive."

---

**Final 60-Second Reading (Read this right before you click 'Join Call')**
"I am completely prepared. I know exactly what SLURM is. It is a workload manager. I have used Terminal and SSH to log into Canadian national supercomputers. I have written C++ code, compiled it on login nodes, and written sbatch shell scripts to ask SLURM for massive amounts of resources—often 192 cores per node. I have used SLURM Job Arrays to launch 20 of these nodes at the exact same time to do parallel math calculations. I know how to check if they fail by using `scancel`, `squeue`, and reading the error `.txt` logs. Ross just wants someone who feels comfortable operating massive computers through a black text screen without breaking things. I have already done this. I am capable. I am ready."
