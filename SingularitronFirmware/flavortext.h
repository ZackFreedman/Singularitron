#ifndef FLAVORTEXTS_H
#define FLAVORTEXTS_H

// The "ing"s are added by the goofy string builder

const char constructiveVerbs[][11] = {
  "Iterat",
  "Synergiz",
  "Calibrat",
  "Instanc",
  "Configur",
  "Snort",
  "Tast",
  "Tweak",
  "Wrangl",
  "Hack",
  "Pwn",
  "Boot",
  "Allocat",
  "Bind",
  "Fund",
  "Ideat",
  "Fabricat",
  "Ping",
  "Refactor",
  "Load",
  "Quantify",
  "Assembl",
  "Distill",
  "Hash",
  "Snuggl",
  "Spoof",
  "Compil",
  "Pressuriz",
  "Chooch",
  "Overrid",
  "Engag",
  "Decrypt",
  "Synthesiz",
  "Appropriat",
  "Analyz",
  "Dispens",
  "Fir",
  "Insert",
  "Align",
  "Fondl",
  "Extrud",
  "Access",
  "Brandish",
  "Enhanc",
  "Planish",
  "Virtualiz",
  "Handcraft",
  "Solv",
  "Augment",
  "Generat",
  "Implement",
  "Download",
  "Construct",
  "Wow! Amaz",
  "Moisten",
  "Gramm",
  "Curat",
  "Buffer",
  "Enter",
  "Induct"
};

#define numberOfConstructiveVerbs 60

const char destructiveVerbs[][11] = {
  "Deallocat",
  "Trash",
  "Smelt",
  "Atomiz",
  "Forgett",
  "Discard",
  "Dropp",
  "Holster",
  "Obsolet",
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
  "Exorcis",
  "Decompil",
  "Emitt",
  "Disengag",
  "Metaboliz",
  "Smash",
  "Encrypt",
  "Crash",
  "Obliterat",
  "Purg",
  "Regrett",
  "Rewind",
  "Free",
  "Delet",
  "Liberat",
  "Retract",
  "Compress",
  "Liquefy",
  "Derezz",
  "Stow",
  "Archiv",
  "Crunch",
  "Analyz",
  "Sync",
  "Throttl",
  "Secur",
  "Withdraw",
  "Regress",
  "Obfuscat",
  "Squirt",
  "Donat",
  "Abandon",
  "Flatten",
  "Tak",
  "Exit",
  "Evacuat",
  "Disrupt",
  "Recycl",
  "Crush"
};

#define numberOfDestructiveVerbs 61

const char nouns[][12] = {
  "the NSA",
  "it",
  "the shmoo",
  "API",
  "BJT man",
  "aesthetics",
  "narrative",
  "tactics",
  "bugs",
  "sauce",
  "everything",
  "data",
  "pixies",
  "AI",
  "phlogiston",
  "spaghetti",
  "amperage",
  "jambalaya",
  "noise",
  "joy",
  "bytecode",
  "anomalies",
  "magic",
  "D-class",
  "bits",
  "phase",
  "flux",
  "sensor",
  "photon",
  "signal",
  "planet",
  "password",
  "chip",
  "privacy",
  "synergy",
  "widget",
  "ion",
  "packet",
  "reality",
  "laser",
  "soul",
  "voltage",
  "register",
  "pun",
  "blockchain",
  "kitten",
  "magic smoke",
  "MacGuffin",
  "core",
  "meme",
  "subroutine",
  "dingus",
  "algorithm",
  "gamma ray",
  "beer",
  "protocol",
  "insult",
  "1337 H4X",
  "black box",
  "unit",
  "excuse",
  "magnet",
  "inductor"
};

#define numberOfNouns 63
#define numberOfPluralNouns 26

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
  output += nouns[nounNumber];

  if (nounNumber > numberOfPluralNouns) {
    if (!output.endsWith('y')) {
      if (output.endsWith('x') || output.endsWith('s')) {
        if (output.length() < 19 && random(2)) output += "es";
      }
      else if (output.length() < 20 && random(2)) output += 's';
    }
  }

  if (output.length() < 18) output += "...";

  output.toCharArray(buffer, 21);

  if (constructive) lastConstructiveVerbNumber = verbNumber;
  else lastDestructiveVerbNumber = verbNumber;
  lastNounNumber = nounNumber;
}

#endif
