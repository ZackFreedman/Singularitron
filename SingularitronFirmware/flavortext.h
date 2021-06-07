#ifndef FLAVORTEXTS_H
#define FLAVORTEXTS_H

//#define PATREON_EASTER_EGG

unsigned int linesGenerated;

// The "ing"s are added by the goofy string builder

const char constructiveVerbs[][11] = {
  "Align",
  "Build",
  "Calibrat",
  "Instanc",
  "Configur",
  "Snort",
  "Microwav",
  "Tweak",
  "Wrangl",
  "Hack",

  "Pwn",
  "Boot",
  "Allocat",
  "Bind",
  "Revv",
  "Polish",
  "Fabricat",
  "Ping",
  "Refactor",
  "Load",

  "Quantify",
  "Assembl",
  "Distill",
  "Bak",
  "Receiv",
  "Unlock",
  "Compil",
  "Pressuriz",
  "Chooch",
  "Mak",

  "Engag",
  "Decrypt",
  "Synthesiz",
  "Predict",
  "Analyz",
  "Dispens",
  "Fir",
  "Insert",
  "Align",
  "Encourag",

  "Extrud",
  "Access",
  "Sharpen",
  "Enhanc",
  "Crank",
  "Stack",
  "Craft",
  "Render",
  "Mount",
  "Generat",

  "Implement",
  "Download",
  "Construct",
  "Wow! Amaz",
  "Moisten",
  "Customiz",
  "Compensat",
  "Buffer",
  "Transferr",
  "Induct",

  "Emitt",
  "Unzipp",
  "Squirt",
  "Feed",
  "Buy",
  "Spark",
  "Implant",
  "Triangulat",
  "Inject",
  "Link",
  "Brew",

  "Process",
  "Deploy",
  "Tun",
  "Attach",
  "Train",
  "Ignor",
  "Tapp",
  "Reload",
  "Simulat",
  "Fluff",

  "Fill",
  "Sort",
  "Updat",
  "Upgrad",
  "Prim",
  "Trac",
  "Inflat",
  "Wangjangl",
  "Charg",
  "Crack",

  "Ignor",
  "Activat",
  "Dial",
  "Pimp",
  "Collect",
  "Approach",
  "Approv",
  "Sampl",
  "Energiz",
  "Stuff"
};

#define numberOfConstructiveVerbs 100

const char destructiveVerbs[][11] = {
  "Deallocat",
  "Trash",
  "Unplugg",
  "Revok",
  "Forgett",
  "Discard",
  "Dropp",
  "Holster",
  "Shredd",
  "Jettison",

  "Dissolv",
  "Liquidat",
  "Releas",
  "Collimat",
  "Eject",
  "Ditch",
  "Leak",
  "Sell",
  "Banish",
  "Dereferenc",

  "Sacrific",
  "Desolder",
  "Destruct",
  "Decompil",
  "Blow",
  "Disengag",
  "Digest",
  "Smash",
  "Encrypt",
  "Crash",

  "Lock",
  "Purg",
  "Regrett",
  "Rewind",
  "Free",
  "Delet",
  "Clos",
  "Retract",
  "Collaps",
  "Liquefy",

  "Derezz",
  "Stow",
  "Archiv",
  "Suspend",
  "Suppress",
  "Clean",
  "Squash",
  "Secur",
  "Withdraw",
  "Dump",

  "Obfuscat",
  "Break",
  "Scrubb",
  "Abandon",
  "Flatten",
  "Stash",
  "Finish",
  "Evacuat",
  "Scrambl",
  "Recycl",

  "Crush",
  "Zipp",
  "Unload",
  "Disconnect",
  "Loosen",
  "Contain",
  "Debat",
  "Detach",
  "Neutraliz",
  "Salvag",

  "Empty",
  "Hid",
  "Disarm",
  "Pickl",
  "Disregard",
  "Yeet",
  "Scrapp",
  "Deflat",
  "Discharg",
  "Deactivat",

  "Steriliz",
  "Reliev",
  "Nuk",
  "Degauss",
  "Dismiss",
  "Drain",
  "Reject",
  "Nerf",
  "Pay",
  "Return",

  "Unstick",
  "Splitt",
  "Cancell",
  "Sham",
  "Embezzl",
  "Fling",
  "Regrett",
  "Halt",
  "Arrest",
  "Bury"
};

#define numberOfDestructiveVerbs 100

#define numberOfNouns 100

const char nouns[numberOfNouns][13] = {
  "content",
  "your mom",
  "the shmoo",
  "API",
  "the BJT man",
  "aesthetics",
  "backstory",
  "tactics",
  "bugs",
  "sauce",

  "warp drive",
  "data",
  "the funk",
  "AI",
  "crystals",
  "spaghetti",
  "fluxgate",
  "electrons",
  "loud noises",
  "wires",

  "bytecode",
  "the truth",
  "magic",
  "hot lava",
  "bits",
  "Brad",
  "Teensy",
  "sensors",
  "photons",
  "signal",

  "the planet",
  "password",
  "chips",
  "circuits",
  "privacy",
  "synergy",
  "widgets",
  "love",
  "packets",
  "reality",

  "lasers",
  "protocols",
  "voltage",
  "registers",
  "puns",
  "dogecoins",
  "kittens",
  "magic smoke",
  "plot device",
  "the core",

  "dank memes",
  "subroutines",
  "radiation",
  "steam",
  "trousers",
  "beer",
  "protocol",
  "one-liners",
  "the Gibson",
  "software",

  "a fat one",
  "holograms",
  "magnets",
  "inductors",
  "resistors",
  "capacitors",
  "viewers",
  "subscribers",
  "sausage",
  "my wife",

  "drama",
  "the future",
  "vectors",
  "the clowns",
  "a Palm Pilot",
  "5G implant",
  "monkeys",
  "breadboard",
  "Patreon",
  "money",

  "the Internet",
  "fluids",
  "the impostor",
  "beats",
  "dopamine",
  "fedora",
  "neural net",
  "comments",
  "ports",
  "you. Yes you",

  "mixtape",
  "[REDACTED]",
  "hot tub",
  "paperwork",
  "Nerf",
  "cyber-doobie",
  "the 1%",
  "the Matrix",
  "variables",
  "IP address"
};

void getFullLine(char * buffer, bool constructive) {
  static int lastConstructiveVerbNumber = numberOfConstructiveVerbs;
  static int lastDestructiveVerbNumber = numberOfDestructiveVerbs;
  static int lastNounNumber = numberOfNouns;

  int verbNumber = random(constructive ? numberOfConstructiveVerbs : numberOfDestructiveVerbs);
  int verbLength = strlen(constructive ? constructiveVerbs[verbNumber] : destructiveVerbs[verbNumber]);
  int nounNumber = random(numberOfNouns);
  int nounLength = strlen(nouns[nounNumber]);

  while ((constructive ? verbNumber == lastConstructiveVerbNumber : verbNumber == lastDestructiveVerbNumber)
         || nounNumber == lastNounNumber
         || verbLength + nounLength > 16) { // We need 4 extra chars for 'ing' and the space
    verbNumber = random(constructive ? numberOfConstructiveVerbs : numberOfDestructiveVerbs);
    verbLength = strlen(constructive ? constructiveVerbs[verbNumber] : destructiveVerbs[verbNumber]);
    nounNumber = random(numberOfNouns);
    nounLength = strlen(nouns[nounNumber]);
  }

  String output = String(constructive ? constructiveVerbs[verbNumber] : destructiveVerbs[verbNumber]);
  output += "ing ";

#ifdef PATREON_EASTER_EGG
  if (linesGenerated % 20 == 0) {
    output += "im noT";
  }
  else if (linesGenerated % 20 == 1) {
    output.remove(0);
    output += "Betta Core...";
  }
  else if (linesGenerated % 20 == 2)
    output += "Weckso...";
  else if (linesGenerated % 20 == 3) {
    output += "Chuck";
  }
  else if (linesGenerated % 20 == 4) {
    output.remove(0);
    output += "FahdooksSmallDong...";
  }
  else if (linesGenerated % 20 == 5)
    output += "cmd...";
  else {
    output += nouns[nounNumber];
    if (output.length() < 18) output += "...";
  }
#endif
#ifndef PATREON_EASTER_EGG
  output += nouns[nounNumber];
  if (output.length() < 18) output += "...";
#endif

  output.toCharArray(buffer, 21);

  if (constructive) lastConstructiveVerbNumber = verbNumber;
  else lastDestructiveVerbNumber = verbNumber;
  lastNounNumber = nounNumber;

  linesGenerated++;
}

#endif

// You can't spell 'flavortext' without 'vortex'
