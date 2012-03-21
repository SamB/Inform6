! ----------------------------------------------------------------------------
!  VERBLIBM:  Core of standard verbs library.
!
!  Supplied for use with Inform 6                         Serial number 970405
!                                                                  Release 6/5
!  (c) Graham Nelson 1993, 1994, 1995, 1996, 1997
!      but freely usable (see manuals)
! ----------------------------------------------------------------------------

#IFDEF MODULE_MODE;
Constant DEBUG;
Constant Grammar__Version2;
Include "linklpa";
Include "linklv";
#ENDIF;

System_file;

! ----------------------------------------------------------------------------
!  The WriteListFrom routine, a flexible object-lister taking care of
!  plurals, inventory information, various formats and so on.  This is used
!  by everything in the library which ever wants to list anything.
!
!  If there were no objects to list, it prints nothing and returns false;
!  otherwise it returns true.
!
!  o is the object, and style is a bitmap, whose bits are given by:
! ----------------------------------------------------------------------------

Constant NEWLINE_BIT    1;    !  New-line after each entry
Constant INDENT_BIT     2;    !  Indent each entry by depth
Constant FULLINV_BIT    4;    !  Full inventory information after entry
Constant ENGLISH_BIT    8;    !  English sentence style, with commas and and
Constant RECURSE_BIT   16;    !  Recurse downwards with usual rules
Constant ALWAYS_BIT    32;    !  Always recurse downwards
Constant TERSE_BIT     64;    !  More terse English style
Constant PARTINV_BIT  128;    !  Only brief inventory information after entry
Constant DEFART_BIT   256;    !  Use the definite article in list
Constant WORKFLAG_BIT 512;    !  At top level (only), only list objects
                              !  which have the "workflag" attribute
Constant ISARE_BIT   1024;    !  Print " is" or " are" before list
Constant CONCEAL_BIT 2048;    !  Omit objects with "concealed" or "scenery":
                              !  if WORKFLAG_BIT also set, then does _not_
                              !  apply at top level, but does lower down
Constant NOARTICLE_BIT 4096;  !  Print no articles, definite or not

[ NextEntry o odepth;
  for(::)
  {   o=sibling(o);
      if (o==0) return 0;
      if (lt_value ~=0 && o.list_together~=lt_value) continue;
      if (c_style & WORKFLAG_BIT ~= 0 && odepth==0 && o hasnt workflag)
          continue;
      if (c_style & CONCEAL_BIT ~= 0 && (o has concealed || o has scenery))
          continue;
      return o;
  }
];

[ WillRecurs o;
  if (c_style & ALWAYS_BIT ~= 0) rtrue;
  if (c_style & RECURSE_BIT == 0) rfalse;
  if (o has transparent
      || o has supporter
      || (o has container && o has open)) rtrue;
  rfalse;
];

[ ListEqual o1 o2;
  if (child(o1)~=0 && WillRecurs(o1)~=0) rfalse;
  if (child(o2)~=0 && WillRecurs(o2)~=0) rfalse;

  if (c_style & (FULLINV_BIT + PARTINV_BIT) ~= 0)
  {   if ((o1 hasnt worn && o2 has worn)
          || (o2 hasnt worn && o1 has worn)) rfalse;
      if ((o1 hasnt light && o2 has light)
          || (o2 hasnt light && o1 has light)) rfalse;
  }

  return Identical(o1,o2);
];

[ SortTogether obj value;
!  print "Sorting together possessions of ",
!         (object) obj, " by value ", value, "^";
!  for (x=child(obj):x~=0:x=sibling(x))
!      print (the) x, " no: ", x, " lt: ", x.list_together, "^";
  while (child(obj)~=0)
  {   if (child(obj).list_together~=value) move child(obj) to out_obj;
      else move child(obj) to in_obj;
  }
  while (child(in_obj)~=0)
      move child(in_obj) to obj;
  while (child(out_obj)~=0)
      move child(out_obj) to obj;
];

[ SortOutList obj i k l;
!  print "^^Sorting out list from ", (name) obj, "^  ";
!  for (i=child(location):i~=0:i=sibling(i))
!      print (name) i, " --> ";
!  new_line;
 .AP_SOL;
  for (i=obj:i~=0:i=sibling(i))
  {   k=i.list_together;
      if (k~=0)
      {   ! print "Scanning ", (name) i, " with lt=", k, "^";
          for (i=sibling(i):i~=0 && i.list_together==k:) i=sibling(i);
              if (i==0) rfalse;
          !print "First not in block is ", (name) i,
          ! " with lt=", i.list_together, "^";
          for (l=sibling(i):l~=0:l=sibling(l))
              if (l.list_together==k)
              {   SortTogether(parent(obj), k);
!  print "^^After ST:^  ";
!  for (i=child(location):i~=0:i=sibling(i))
!      print (name) i, " --> ";
!  new_line;
                  obj = child(parent(obj));
                  jump AP_SOL;
              }
      }
  }
];

[ Print__Spaces n;         ! To avoid a bug occurring in Inform 6.01 to 6.10
  if (n==0) return; spaces n; ];

[ WriteListFrom o style depth;
  if (o==child(parent(o)))
  {   SortOutList(o); o=child(parent(o)); }
  c_style=style;
  wlf_indent=0; WriteListR(o,depth);
  rtrue;
];

[ WriteListR o depth stack_pointer  classes_p sizes_p i j k k2 l m n q senc mr;

  if (depth>0 && o==child(parent(o)))
  {   SortOutList(o); o=child(parent(o)); }
  for (::)
  {   if (o==0) rfalse;
      if (c_style & WORKFLAG_BIT ~= 0 && depth==0 && o hasnt workflag)
      {   o = sibling(o); continue; }
      if (c_style & CONCEAL_BIT ~= 0
          && (o has concealed || o has scenery))
      {   o=sibling(o); continue; }
      break;
  }

  classes_p = match_classes + stack_pointer;
  sizes_p   = match_list + stack_pointer;

  for (i=o,j=0:i~=0 && (j+stack_pointer)<128:i=NextEntry(i,depth),j++)
  {   classes_p->j=0;
      if (i.plural~=0) k++;
  }

  if (c_style & ISARE_BIT ~= 0)
  {   if (j==1 && o hasnt pluralname)
          print (string) IS__TX; else print (string) ARE__TX;
      if (c_style & NEWLINE_BIT ~= 0) print ":^"; else print (char) ' ';
      c_style = c_style - ISARE_BIT;
  }

  stack_pointer = stack_pointer+j+1;

  if (k<2) jump EconomyVersion;   ! It takes two to plural

  n=1;
  for (i=o,k=0:k<j:i=NextEntry(i,depth),k++)
      if (classes_p->k==0)
      {   classes_p->k=n; sizes_p->n=1;
          for (l=NextEntry(i,depth), m=k+1:l~=0 && m<j:
               l=NextEntry(l,depth), m++)
              if (classes_p->m==0 && i.plural~=0 && l.plural~=0)
              {   if (ListEqual(i,l)==1)
                  {   sizes_p->n = sizes_p->n + 1;
                      classes_p->m = n;
                  }
              }
          n++;
      }
  n--;

  for (i=1, j=o, k=0: i<=n: i++, senc++)
  {   while (((classes_p->k) ~= i)
             && ((classes_p->k) ~= -i)) { k++; j=NextEntry(j,depth); }
      m=sizes_p->i;
      if (j.list_together~=0 or lt_value
          && ZRegion(j.list_together)==2 or 3
          && j.list_together==mr) senc--;
      mr=j.list_together;
  }
  senc--;
  for (i=1, j=o, k=0, mr=0: senc>=0: i++, senc--)
  {   while (((classes_p->k) ~= i)
             && ((classes_p->k) ~= -i)) { k++; j=NextEntry(j,depth); }
      if (j.list_together~=0 or lt_value)
      {   if (j.list_together==mr) { senc++; jump Omit_FL2; }
          k2=NextEntry(j,depth);
          if (k2==0 || k2.list_together~=j.list_together) jump Omit_WL2;
          k2=ZRegion(j.list_together);
          if (k2==2 or 3)
          {   q=j; listing_size=1; l=k; m=i;
              while (m<n && q.list_together==j.list_together)
              {   m++;
                  while (((classes_p->l) ~= m)
                         && ((classes_p->l) ~= -m))
                  {   l++; q=NextEntry(q,depth); }
                  if (q.list_together==j.list_together) listing_size++;
              }
!              print " [", listing_size, "] ";
              if (listing_size==1) jump Omit_WL2;
              if (c_style & INDENT_BIT ~= 0)
                  Print__Spaces(2*(depth+wlf_indent));
              if (k2==3)
              {   q=0; for (l=0:l<listing_size:l++) q=q+sizes_p->(l+i);
                  EnglishNumber(q); print " ";
                  print (string) j.list_together;
                  if (c_style & ENGLISH_BIT ~= 0) print " (";
                  if (c_style & INDENT_BIT ~= 0) print ":^";
              }
              q=c_style;
              if (k2~=3)
              {   inventory_stage=1;
                  parser_one=j; parser_two=depth+wlf_indent;
                  if (RunRoutines(j,list_together)==1) jump Omit__Sublist2;
              }
              lt_value=j.list_together; listing_together=j; wlf_indent++;
              WriteListR(j,depth,stack_pointer); wlf_indent--;
              lt_value=0; listing_together=0;
              if (k2==3)
              {   if (q & ENGLISH_BIT ~= 0) print ")";
              }
              else
              {   inventory_stage=2;
                  parser_one=j; parser_two=depth+wlf_indent;
                  RunRoutines(j,list_together);
              }
             .Omit__Sublist2;
              if (q & NEWLINE_BIT ~= 0 && c_style & NEWLINE_BIT == 0)
                  new_line;
              c_style=q;
              mr=j.list_together;
              jump Omit_EL2;
          }
      }

     .Omit_WL2;
      if (WriteBeforeEntry(j,depth)==1) jump Omit_FL2;
      if (sizes_p->i == 1)
      {   if (c_style & NOARTICLE_BIT ~= 0) print (name) j;
          else
          {   if (c_style & DEFART_BIT ~= 0) print (the) j; else print (a) j;
          }
      }
      else
      {   if (c_style & DEFART_BIT ~= 0)
              PrefaceByArticle(j, 1, sizes_p->i);
          print (number) sizes_p->i, " ";
          PrintOrRun(j,plural,1);
      }
      WriteAfterEntry(j,depth,stack_pointer);

     .Omit_EL2;
      if (c_style & ENGLISH_BIT ~= 0)
      {   if (senc==1) print (string) AND__TX;
          if (senc>1) print ", ";
      }
     .Omit_FL2;
  }
  rtrue;

  .EconomyVersion;

  n=j;

  for (i=1, j=o: i<=n: j=NextEntry(j,depth), i++, senc++)
  {   if (j.list_together~=0 or lt_value
          && ZRegion(j.list_together)==2 or 3
          && j.list_together==mr) senc--;
      mr=j.list_together;
  }

  for (i=1, j=o, mr=0: i<=senc: j=NextEntry(j,depth), i++)
  {   if (j.list_together~=0 or lt_value)
      {   if (j.list_together==mr) { i--; jump Omit_FL; }
          k=NextEntry(j,depth);
          if (k==0 || k.list_together~=j.list_together) jump Omit_WL;
          k=ZRegion(j.list_together);
          if (k==2 or 3)
          {   if (c_style & INDENT_BIT ~= 0)
                  Print__Spaces(2*(depth+wlf_indent));
              if (k==3)
              {   q=j; l=0;
                  do
                  {   q=NextEntry(q,depth); l++;
                  } until (q.list_together~=j.list_together);
                  EnglishNumber(l); print " ";
                  print (string) j.list_together;
                  if (c_style & ENGLISH_BIT ~= 0) print " (";
                  if (c_style & INDENT_BIT ~= 0) print ":^";
              }
              q=c_style;
              if (k~=3)
              {   inventory_stage=1;
                  parser_one=j; parser_two=depth+wlf_indent;
                  if (RunRoutines(j,list_together)==1) jump Omit__Sublist;
              }
              lt_value=j.list_together; listing_together=j; wlf_indent++;
              WriteListR(j,depth,stack_pointer); wlf_indent--;
              lt_value=0; listing_together=0;
              if (k==3)
              {   if (q & ENGLISH_BIT ~= 0) print ")";
              }
              else
              {   inventory_stage=2;
                  parser_one=j; parser_two=depth+wlf_indent;
                  RunRoutines(j,list_together);
              }
             .Omit__Sublist;
              if (q & NEWLINE_BIT ~= 0 && c_style & NEWLINE_BIT == 0) new_line;
              c_style=q;
              mr=j.list_together;
              jump Omit_EL;
          }
      }
     .Omit_WL;
      if (WriteBeforeEntry(j,depth)==1) jump Omit_FL;
      if (c_style & NOARTICLE_BIT ~= 0) print (name) j;
      else
      {   if (c_style & DEFART_BIT ~= 0) print (the) j; else print (a) j;
      }
      WriteAfterEntry(j,depth,stack_pointer);

     .Omit_EL;
      if (c_style & ENGLISH_BIT ~= 0)
      {   if (i==senc-1) print (string) AND__TX;
          if (i<senc-1) print ", ";
      }
     .Omit_FL;
  }
];

[ WriteBeforeEntry o depth  flag;
  if (c_style & INDENT_BIT ~= 0) Print__Spaces(2*(depth+wlf_indent));

  if (c_style & FULLINV_BIT ~= 0)
  {   if (o.invent~=0)
      {   inventory_stage=1;
          flag=PrintOrRun(o,invent,1);
          if (flag==1 && c_style & NEWLINE_BIT ~= 0) new_line;
      }
  }
  return flag;
];

[ WriteAfterEntry o depth stack_p  flag flag2 flag3 p comb;

  if (c_style & PARTINV_BIT ~= 0)
  {   comb=0;
      if (o has light && location hasnt light) comb=comb+1;
      if (o has container && o hasnt open)     comb=comb+2;
      if ((o has container && (o has open || o has transparent))
          && (child(o)==0)) comb=comb+4;
      if (comb==1) L__M(##ListMiscellany, 1, o);
      if (comb==2) L__M(##ListMiscellany, 2, o);
      if (comb==3) L__M(##ListMiscellany, 3, o);
      if (comb==4) L__M(##ListMiscellany, 4, o);
      if (comb==5) L__M(##ListMiscellany, 5, o);
      if (comb==6) L__M(##ListMiscellany, 6, o);
      if (comb==7) L__M(##ListMiscellany, 7, o);
  }

  if (c_style & FULLINV_BIT ~= 0)
  {   if (o.invent ~= 0)
      {   inventory_stage=2;
          if (RunRoutines(o,invent)~=0)
          {   if (c_style & NEWLINE_BIT ~= 0) new_line;
              rtrue;
          }
      }
      if (o has light && o has worn)
      {    L__M(##ListMiscellany, 8); flag2=1; }
      else
      {   if (o has light) {  L__M(##ListMiscellany, 9, o); flag2=1; }
          if (o has worn)  {  L__M(##ListMiscellany, 10, o); flag2=1; }
      }
      if (o has container)
      {   if (o has openable)
          {   if (flag2==1) print (string) AND__TX;
              else L__M(##ListMiscellany, 11, o);
              if (o has open)
              {   if (child(o)==0) L__M(##ListMiscellany, 13, o);
                  else L__M(##ListMiscellany, 12, o);
              }
              else
              {   if (o has lockable && o has locked)
                      L__M(##ListMiscellany, 15, o);
                  else L__M(##ListMiscellany, 14, o);
              }
              flag2=1;
          }
          else
              if (child(o)==0 && o has transparent)
              {   if (flag2==1) L__M(##ListMiscellany, 16, o);
                  else L__M(##ListMiscellany, 17, o);
              }
      }
      if (flag2==1) print ")";
  }

  if (c_style & CONCEAL_BIT == 0)
  {   flag3 = children(o);
      flag2 = child(o);
  }
  else
  {   flag3 = 0;
      objectloop (p in o)
          if (p hasnt concealed && p hasnt scenery) { flag3++; flag2 = p; }
  }

  if (c_style & ALWAYS_BIT ~= 0 && flag3>0)
  {   if (c_style & ENGLISH_BIT ~= 0) L__M(##ListMiscellany, 18, o);
      flag=1;
  }

  if (c_style & RECURSE_BIT ~= 0 && flag3>0)
  {   if (o has supporter)
      {   if (c_style & ENGLISH_BIT ~= 0)
          {   if (c_style & TERSE_BIT ~= 0)
                   L__M(##ListMiscellany, 19, o);
              else L__M(##ListMiscellany, 20, o);
              if (o has animate) print (string) WHOM__TX;
              else print (string) WHICH__TX;
          }
          flag=1;
      }
      if (o has container && (o has open || o has transparent))
      {   if (c_style & ENGLISH_BIT ~= 0)
          {   if (c_style & TERSE_BIT ~= 0)
                   L__M(##ListMiscellany, 21, o);
              else L__M(##ListMiscellany, 22, o);
              if (o has animate) print (string) WHOM__TX;
              else print (string) WHICH__TX;
          }
          flag=1;
      }
  }

  if (flag==1 && c_style & ENGLISH_BIT ~= 0)
  {   if (flag3 > 1 || flag2 has pluralname)
           print (string) ARE2__TX;
      else print (string) IS2__TX;
  }

  if (c_style & NEWLINE_BIT ~= 0) new_line;

  if (flag==1)
  {   o = child(o);
      WriteListR(o, depth+1, stack_p);
      if (c_style & TERSE_BIT ~= 0) print ")";
  }
];

! ----------------------------------------------------------------------------
!   A cunning routine (which could have been a daemon, but isn't, for the
!   sake of efficiency) to move objects which could be in many rooms about
!   so that the player never catches one not in place
! ----------------------------------------------------------------------------

[ MoveFloatingObjects i k l m address;
  objectloop (i)
  {   address=i.&found_in;
      if (address~=0 && i hasnt absent)
      {   if (ZRegion(address-->0)==2)
          {   if (indirect(address-->0) ~= 0) move i to location;
          }
          else
          {   k=i.#found_in;
              for (l=0: l<k/2: l++)
              {   m=address-->l;
                  if (m==location || m in location) move i to location;
              }
          }
      }
  }
];

! ----------------------------------------------------------------------------
!   Two little routines for moving the player safely.
! ----------------------------------------------------------------------------

[ PlayerTo newplace flag;
  move player to newplace;
  while (parent(newplace)~=0) newplace=parent(newplace);
  location=newplace;
  real_location=location;
  AdjustLight(1);
  if (flag==0) <Look>;
  if (flag==1) { NoteArrival(); ScoreArrival(); }
  if (flag==2) LookSub(1);
];

[ MovePlayer direc; <Go direc>; <Look>; ];

! ----------------------------------------------------------------------------
!   The handy YesOrNo routine, and some "meta" verbs
! ----------------------------------------------------------------------------

[ YesOrNo i;
  for (::)
  {
   #IFV3; read buffer parse; #ENDIF;
   #IFV5; read buffer parse DrawStatusLine; #ENDIF;
      i=parse-->1;
      if (i==YES1__WD or YES2__WD or YES3__WD) rtrue;
      if (i==NO1__WD or NO2__WD or NO3__WD) rfalse;
      L__M(##Quit,1); print "> ";
  }
];

[ QuitSub; L__M(##Quit,2); if (YesOrNo()~=0) quit; ];

[ RestartSub; L__M(##Restart,1);
  if (YesOrNo()~=0) { @restart; L__M(##Restart,2); }
];

[ RestoreSub;
  restore Rmaybe;
  return L__M(##Restore,1);
  .RMaybe; L__M(##Restore,2);
];

[ SaveSub;
  save Smaybe;
  return L__M(##Save,1);
  .SMaybe; L__M(##Save,2);
];

[ VerifySub;
  @verify ?Vmaybe;
  jump Vwrong;
  .Vmaybe; return L__M(##Verify,1);
  .Vwrong;
  L__M(##Verify,2);
];

[ ScriptOnSub;
  if (transcript_mode==1) return L__M(##ScriptOn,1);
  transcript_mode=1;
  0-->8 = (0-->8)|1;
  L__M(##ScriptOn,2); VersionSub();
];

[ ScriptOffSub;
  if (transcript_mode==0) return L__M(##ScriptOff,1);
  L__M(##ScriptOff,2);
  transcript_mode=0;
  0-->8 = (0-->8)&$fffe;
];

[ NotifyOnSub; notify_mode=1; L__M(##NotifyOn); ];
[ NotifyOffSub; notify_mode=0; L__M(##NotifyOff); ];

[ Places1Sub i j k;
  L__M(##Places);
  objectloop(i has visited) j++;

  objectloop(i has visited)
  {   print (name) i; k++;
      if (k==j) ".";
      if (k==j-1) print (string) AND__TX; else print ", ";
  }
];
[ Objects1Sub i j f;
  L__M(##Objects,1);
  objectloop(i has moved)
  {   f=1; print (the) i; j=parent(i);
      if (j==player)
      {   if (i has worn) L__M(##Objects, 3);
          else L__M(##Objects, 4);
          jump obj__ptd;
      }

      if (j has animate)   { L__M(##Objects, 5); jump obj__ptd; }
      if (j has visited)   { L__M(##Objects, 6, j); jump obj__ptd; }
      if (j has container) { L__M(##Objects, 8, j); jump obj__ptd; }
      if (j has supporter) { L__M(##Objects, 9, j); jump obj__ptd; }
      if (j has enterable) { L__M(##Objects, 7, j); jump obj__ptd; }

      L__M(##Objects, 10);
      .obj__ptd; new_line;
  }
  if (f==0) L__M(##Objects,2);
];

! ----------------------------------------------------------------------------
!   The scoring system
! ----------------------------------------------------------------------------

[ ScoreSub;
  L__M(##Score);
  PrintRank();
];

[ Achieved num;
  if (task_done->num==0)
  {   task_done->num=1;
      score = score + task_scores->num;
  }
];

[ PANum m n;
  print "  ";
  n=m;
  if (n<0)    { n=-m; n=n*10; }
  if (n<10)   { print "   "; jump panuml; }
  if (n<100)  { print "  "; jump panuml; }
  if (n<1000) { print " "; }
.panuml;
  print m, " ";
];

[ FullScoreSub i;
  ScoreSub();
  if (score==0 || TASKS_PROVIDED==1) rfalse;
  new_line;
  L__M(##FullScore,1);

  for (i=0:i<NUMBER_TASKS:i++)
      if (task_done->i==1)
      {   PANum(task_scores->i);
          PrintTaskName(i);
      }
  
  if (things_score~=0)
  {   PANum(things_score); L__M(##FullScore,2); }
  if (places_score~=0)
  {   PANum(places_score); L__M(##FullScore,3); }
  new_line; PANum(score); L__M(##FullScore,4);
];

! ----------------------------------------------------------------------------
!   Real verbs start here: Inventory
! ----------------------------------------------------------------------------

[ InvWideSub;
  inventory_style = FULLINV_BIT + ENGLISH_BIT + RECURSE_BIT;
  <Inv>;
];

[ InvTallSub;
  inventory_style = FULLINV_BIT + INDENT_BIT + NEWLINE_BIT + RECURSE_BIT;
  <Inv>;
];

[ InvSub x;
  if (child(player)==0) return L__M(##Inv,1);
  if (inventory_style==0) return InvTallSub();

  L__M(##Inv,2);
  if (inventory_style & NEWLINE_BIT ~= 0) print ":^"; else print " ";

  WriteListFrom(child(player), inventory_style, 1);
  if (inventory_style & ENGLISH_BIT ~= 0) print ".^";

  objectloop(x in player) PronounNotice(x);

  AfterRoutines();
];

! ----------------------------------------------------------------------------
!   The object tree and determining the possibility of moves
! ----------------------------------------------------------------------------

[ CommonAncestor o1 o2 i j;
  !  Find the nearest object indirectly containing o1 and o2,
  !  or return 0 if there is no common ancestor.

  i = o1;
  while (i ~= 0)
  {
      j = o2;
      while (j ~= 0)
      {   if (j == i) return i;
          j = parent(j);
      }
      i = parent(i);
  }
  return 0;
];

[ IndirectlyContains o1 o2;
  !  Does o1 indirectly contain o2?  (Same as testing if their common
  !  ancestor is o1.)

  while (o2~=0)
  {   if (o1==o2) rtrue;
      o2=parent(o2);
  }
  rfalse;
];

[ ObjectIsUntouchable item flag1 flag2 ancestor i;

  ! Determine if there's any barrier preventing the player from moving
  ! things to "item".  Return false if no barrier; otherwise print a
  ! suitable message and return true.
  ! If flag1 is set, do not print any message.
  ! If flag2 is set, also apply Take/Remove restrictions.

  ancestor = CommonAncestor(player, item);

  ! First, a barrier between the player and the ancestor.  The player
  ! can only be in a sequence of enterable objects, and only closed
  ! containers form a barrier.

  if (player ~= ancestor)
  {   i = parent(player);
      while (i~=ancestor)
      {   if (i has container && i hasnt open)
          {   if (flag1) rtrue;
              return L__M(##Take,9,i);
          }
          i = parent(i);
      }
  }

  ! Second, a barrier between the item and the ancestor.  The item can
  ! be carried by someone, part of a piece of machinery, in or on top
  ! of something and so on.

  if (item ~= ancestor)
  {   i = parent(item);
      while (i~=ancestor)
      {   if (flag2 && i hasnt container && i hasnt supporter)
          {   if (i has animate)
              {   if (flag1) rtrue;
                  return L__M(##Take,6,i);
              }
              if (i has transparent)
              {   if (flag1) rtrue;
                  return L__M(##Take,7,i);
              }
              if (flag1) rtrue;
              return L__M(##Take,8,item);
          }
          if (i has container && i hasnt open)
          {   if (flag1) rtrue;
              return L__M(##Take,9,i);
          }
          i = parent(i);
      }
  }
  rfalse;
];

[ AttemptToTakeObject item     ancestor after_recipient i j k;
  ! Try to transfer the given item to the player: return false
  ! if successful, true if unsuccessful, printing a suitable message
  ! in the latter case.

  ! People cannot ordinarily be taken.
  if (item == player) return L__M(##Take,2);
  if (item has animate) return L__M(##Take,3,item);

  ancestor = CommonAncestor(player, item);

  ! Are player and item in totally different places?
  if (ancestor == 0) return L__M(##Take,8,item);

  ! Is the player indirectly inside the item?
  if (ancestor == item) return L__M(##Take,4,item);

  ! Does the player already directly contain the item?
  if (item in player) return L__M(##Take,5,item);

  ! Can the player touch the item, or is there (e.g.) a closed container
  ! in the way?
  if (ObjectIsUntouchable(item,false,true)) return;

  ! The item is now known to be accessible.

  ! Consult the immediate possessor of the item, if it's in a container
  ! which the player is not in.

  i=parent(item);
  if (i ~= ancestor && (i has container || i has supporter))
  {   after_recipient=i;
      k=action; action=##LetGo;
      if (RunRoutines(i,before)~=0) { action=k; rtrue; }
      action=k;
  }

  if (item has scenery) return L__M(##Take,10,item);
  if (item has static)  return L__M(##Take,11,item);

  ! The item is now known to be available for taking.  Is the player
  ! carrying too much?  If so, possibly juggle items into the rucksack
  ! to make room.

  k=0; objectloop (j in player) if (j hasnt worn) k++;

  if (k >= ValueOrRun(player,capacity))
  {   if (SACK_OBJECT~=0)
      {   if (parent(SACK_OBJECT)~=player)
              return L__M(##Take,12);
          j=0;
          objectloop (k in player) 
              if (k~=SACK_OBJECT && k hasnt worn && k hasnt light) j=k;

          if (j~=0)
          {   L__M(##Take,13,j);
              keep_silent = 1; <Insert j SACK_OBJECT>; keep_silent = 0;
              if (j notin SACK_OBJECT) rtrue;
          }
          else return L__M(##Take,12);
      }
      else return L__M(##Take,12);
  }

  ! Transfer the item.

  move item to player;

  ! Send "after" message to the object letting go of the item, if any.

  if (after_recipient~=0)
  {   k=action; action=##LetGo;
      if (RunRoutines(after_recipient,after)~=0) { action=k; rtrue; }
      action=k;
  }
  rfalse;
];

! ----------------------------------------------------------------------------
!   Object movement verbs
! ----------------------------------------------------------------------------

[ TakeSub;
  if (onotheld_mode==0 || noun notin player)
      if (AttemptToTakeObject(noun)) rtrue;
  if (AfterRoutines()==1) rtrue;
  notheld_mode=onotheld_mode;
  if (notheld_mode==1 || keep_silent==1) rtrue;
  L__M(##Take,1);
];

[ RemoveSub i;
  i=parent(noun);
  if (i has container && i hasnt open) return L__M(##Remove,1,noun);
  if (i~=second) return L__M(##Remove,2,noun);
  if (i has animate) return L__M(##Take,6,i);
  if (AttemptToTakeObject(noun)) rtrue;
  action=##Take;   if (AfterRoutines()==1) rtrue;
  action=##Remove; if (AfterRoutines()==1) rtrue;

  if (keep_silent==1) rtrue;
  return L__M(##Remove,3,noun);
];

[ DropSub;
  if (noun in parent(player)) return L__M(##Drop,1,noun);
  if (noun notin player) return L__M(##Drop,2,noun);
  if (noun has worn)
  {   L__M(##Drop,3,noun);
      <Disrobe noun>;
      if (noun has worn) rtrue;
  }
  move noun to parent(player);
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  return L__M(##Drop,4,noun);
];

[ PutOnSub ancestor;
  receive_action=##PutOn; 
  if (second == d_obj || player in second) <<Drop noun>>;
  if (parent(noun)~=player) return L__M(##PutOn,1,noun);

  ancestor = CommonAncestor(noun, second);
  if (ancestor == noun) return L__M(##PutOn,2,noun);
  if (ObjectIsUntouchable(second)) return;

  if (second ~= ancestor)
  {   action=##Receive;
      if (RunRoutines(second,before)~=0) { action=##PutOn; return; }
      action=##PutOn;
  }
  if (second hasnt supporter) return L__M(##PutOn,3,second);
  if (ancestor == player) return L__M(##PutOn,4);
  if (noun has worn)
  {   L__M(##PutOn,5,noun); <Disrobe noun>; if (noun has worn) return;
  }

  if (children(second)>=ValueOrRun(second,capacity))
      return L__M(##PutOn,6,second);

  move noun to second;

  if (AfterRoutines()==1) return;

  if (second ~= ancestor)
  {   action=##Receive;
      if (RunRoutines(second,after)~=0) { action=##PutOn; return; }
      action=##PutOn;
  }
  if (keep_silent==1) return;
  if (multiflag==1) return L__M(##PutOn,7);
  L__M(##PutOn,8,noun);
];

[ InsertSub ancestor;
  receive_action = ##Insert;
  if (second==d_obj || player in second) <<Drop noun>>;
  if (parent(noun)~=player) return L__M(##Insert,1,noun);

  ancestor = CommonAncestor(noun, second);
  if (ancestor == noun) return L__M(##Insert, 5, noun);
  if (ObjectIsUntouchable(second)) return;

  if (second ~= ancestor)
  {   action=##Receive;
      if (RunRoutines(second,before)~=0) { action=##Insert; rtrue; }
      action=##Insert;
      if (second has container && second hasnt open)
          return L__M(##Insert,3,second);
  }
  if (second hasnt container) return L__M(##Insert,2,second);

  if (noun has worn)
  {   L__M(##Insert,6,noun); <Disrobe noun>; if (noun has worn) return;
  }

  if (children(second) >= ValueOrRun(second,capacity))
      return L__M(##Insert,7,second);

  move noun to second;

  if (AfterRoutines()==1) rtrue;

  if (second ~= ancestor)
  {   action=##Receive;
      if (RunRoutines(second,after)~=0) { action=##Insert; rtrue; }
      action=##Insert;
  }
  if (keep_silent==1) rtrue;
  if (multiflag==1) return L__M(##Insert,8,noun);
  L__M(##Insert,9,noun);
];

! ----------------------------------------------------------------------------
!   Empties and transfers are routed through the actions above
! ----------------------------------------------------------------------------

[ TransferSub;
  if (noun notin player && AttemptToTakeObject(noun)) return;
  if (second has container) <<Insert noun second>>;
  if (second has supporter) <<PutOn noun second>>;
  <<Drop noun>>;
];

[ EmptySub;
  second=d_obj; EmptyTSub();
];

[ EmptyTSub i j;
  if (ObjectIsUntouchable(noun)) return;
  if (noun hasnt container) return L__M(##EmptyT,1,noun);
  if (noun hasnt open) return L__M(##EmptyT,2,noun);
  if (second~=d_obj)
  {   if (second hasnt container) return L__M(##EmptyT,1,second);
      if (second hasnt open) return L__M(##EmptyT,2,second);
  }
  i=child(noun);
  if (i==0) return L__M(##EmptyT,3,noun);
  while (i~=0)
  {   j=sibling(i); print (name) i, ": ";
      <Transfer i second>;
      i=j;
  }
];

! ----------------------------------------------------------------------------
!   Gifts
! ----------------------------------------------------------------------------

[ GiveSub;
  if (parent(noun)~=player) return L__M(##Give,1,noun);
  if (second==player)  return L__M(##Give,2,noun);
  if (RunLife(second,##Give)~=0) rfalse;
  L__M(##Give,3,second);
];

[ GiveRSub; <Give second noun>; ];

[ ShowSub;
  if (parent(noun)~=player) return L__M(##Show,1,noun);
  if (second==player) <<Examine noun>>;
  if (RunLife(second,##Show)~=0) rfalse;
  L__M(##Show,2,second);
];

[ ShowRSub; <Show second noun>; ];

! ----------------------------------------------------------------------------
!   Travelling around verbs
! ----------------------------------------------------------------------------

[ EnterSub ancestor j k;
  if (noun has door || noun in compass) <<Go noun>>;

  if (player in noun) return L__M(##Enter,1,noun);
  if (noun hasnt enterable) return L__M(##Enter,2,noun);
  if (noun has container && noun hasnt open) return L__M(##Enter,3,noun);

  if (parent(player) ~= parent(noun))
  {   ancestor = CommonAncestor(player, noun);
      if (ancestor == player or 0) return L__M(##Enter,4,noun);
      while (player notin ancestor)
      {   j = parent(player);
          if (parent(j) ~= ancestor || noun ~= ancestor)
          {   L__M(##Enter,6,j);
              k = keep_silent; keep_silent = 1;
          }    
          <Exit>;
          keep_silent = k;
          if (player in j) return;
      }
      if (player in noun) return;
      if (noun notin ancestor)
      {   j = parent(noun);
          while (parent(j) ~= ancestor) j = parent(j);
          L__M(##Enter,7,j);
          k = keep_silent; keep_silent = 1;
          <Enter j>;
          keep_silent = k;
          if (player notin j) return;
          <<Enter noun>>;
      }
  }

  move player to noun;
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  L__M(##Enter,5,noun);
  Locale(noun);
];

[ GetOffSub;
  if (parent(player)==noun) <<Exit>>;
  L__M(##GetOff,1,noun);
];

[ ExitSub p;
  p=parent(player);
  if (p==location || (location==thedark && p==real_location))
  {   if ((location.out_to~=0)
          || (location==thedark && real_location.out_to~=0)) <<Go out_obj>>;
      return L__M(##Exit,1);
  }
  if (p has container && p hasnt open)
      return L__M(##Exit,2,p);

  move player to parent(p);

  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  L__M(##Exit,3,p); LookSub(1);
];

[ VagueGoSub; L__M(##VagueGo); ];

[ GoInSub;
  <<Go in_obj>>;
];

[ GoSub i j k df movewith thedir;

  movewith=0;
  i=parent(player);
  if ((location~=thedark && i~=location)
      || (location==thedark && i~=real_location))
  {   j=location;
      if (location==thedark) location=real_location;
      k=RunRoutines(i,before); if (k~=3) location=j;
      if (k==1)
      {   movewith=i; i=parent(i); jump gotroom; }
      if (k==0) L__M(##Go,1,i); rtrue;
  }
  .gotroom;
  thedir=noun.door_dir;
  if (ZRegion(thedir)==2) thedir=RunRoutines(noun,door_dir);
  
  j=i.thedir; k=ZRegion(j);
  if (k==3) { print (string) j; new_line; rfalse; }
  if (k==2) { j=RunRoutines(i,thedir);
              if (j==1) rtrue;
            }

  if (k==0 || j==0)
  {   if (i.cant_go ~= 0) PrintOrRun(i, cant_go);
      rfalse;
  }

  if (j has door)
  {   if (j has concealed) return L__M(##Go,2);
      if (j hasnt open)
      {   if (noun==u_obj) return L__M(##Go,3,j);
          if (noun==d_obj) return L__M(##Go,4,j);
          return L__M(##Go,5,j);
      }
      if (ZRegion(j.door_to)==2) j=RunRoutines(j,door_to);
      else
      {   if (j.door_to == 0) return L__M(##Go,6,j);
          j=j.door_to;
      }
      if (j==1) rtrue;
  }
  if (movewith==0) move player to j; else move movewith to j;

  df=OffersLight(j);
  if (df~=0) { location=j; lightflag=1; }
  else
  {   if (location==thedark)
      {   DarkToDark();
          if (deadflag~=0) rtrue;
      }
      real_location=j;
      location=thedark; lightflag=0;
  }
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  LookSub(1);
];

! ----------------------------------------------------------------------------
!   Describing the world.  SayWhatsOn(object) does just that (producing
!   no text if nothing except possibly "scenery" and "concealed" items are).
!   Locale(object) runs through the "tail end" of a Look-style room
!   description for the contents of the object, printing up suitable
!   descriptions as it goes.
! ----------------------------------------------------------------------------

[ SayWhatsOn descon j f;
  if (descon==parent(player)) rfalse;
  objectloop (j in descon)
      if (j hasnt concealed && j hasnt scenery) f=1;
  if (f==0) rfalse;
  L__M(##Look, 4, descon); rtrue;
];

[ Locale descin text1 text2  o p k j flag f2;

  objectloop (o in descin) give o ~workflag;

  k=0;
  objectloop (o in descin)
      if (o hasnt concealed && o~=parent(player))
      {  PronounNotice(o);
         if (o hasnt scenery)
         {   give o workflag; k++;
             p=initial; f2=0;
             if ((o has door || o has container)
                 && o has open && o provides when_open)
             {   p = when_open; f2 = 1; jump Prop_Chosen; }
             if ((o has door || o has container)
                 && o hasnt open && o provides when_closed)
             {   p = when_closed; f2 = 1; jump Prop_Chosen; }
             if (o has switchable
                 && o has on && o provides when_on)
             {   p = when_on; f2 = 1; jump Prop_Chosen; }
             if (o has switchable
                 && o hasnt on && o provides when_off)
             {   p = when_off; f2 = 1; }

             .Prop_Chosen;

             if (o hasnt moved || o.describe~=NULL || f2==1)
             {   if (o.describe~=NULL && RunRoutines(o,describe)~=0)
                 {   flag=1;
                     give o ~workflag; k--;
                 }    
                 else
                 {   j=o.p;
                     if (j~=0)
                     {   new_line;
                         PrintOrRun(o,p);
                         flag=1;
                         give o ~workflag; k--;
                         if (o has supporter && child(o)~=0) SayWhatsOn(o);
                     }
                 }
             }
         }
         else
             if (o has supporter && child(o)~=0) SayWhatsOn(o);
      }

  if (k==0) return 0;

  if (text1~=0)
  {   new_line;
      if (flag==1) text1=text2;
      print (string) text1, " ";
      WriteListFrom(child(descin),
          ENGLISH_BIT + WORKFLAG_BIT + RECURSE_BIT
          + PARTINV_BIT + TERSE_BIT + CONCEAL_BIT);
      return k;
  }
           
  if (flag==1) L__M(##Look,5,descin); else L__M(##Look,6,descin);
];

! ----------------------------------------------------------------------------
!   Looking.  LookSub(1) is allowed to abbreviate long descriptions, but
!     LookSub(0) (which is what happens when the Look action is generated)
!     isn't.  (Except that these are over-ridden by the player-set lookmode.)
! ----------------------------------------------------------------------------

[ LMode1Sub; lookmode=1; print (string) Story; L__M(##LMode1); ];  ! Brief

[ LMode2Sub; lookmode=2; print (string) Story; L__M(##LMode2); ];  ! Verbose

[ LMode3Sub; lookmode=3; print (string) Story; L__M(##LMode3); ];  ! Superbrief

[ NoteArrival descin;
  descin=location;
  if (descin~=lastdesc)
  {   if (descin.initial~=0) PrintOrRun(descin, initial);
      NewRoom();
      MoveFloatingObjects();
      lastdesc=descin;
  }
];

[ ScoreArrival;
  if (location hasnt visited)
  {   give location visited;
      if (location has scored)
      {   score = score + ROOM_SCORE;
          places_score = places_score + ROOM_SCORE;
      }
  }
];

[ LookSub allow_abbrev  visible visibility_levels i j k;
  if (parent(player)==0) return RunTimeError(10);

  if (location == thedark) visible = thedark;
  else
  {   visibility_levels = 1;
      visible = parent(player);
      while ((parent(visible) ~= 0)
             && (visible hasnt container
                 || visible has open || visible has transparent))
      {   visible = parent(visible);
          visibility_levels++;
      }
      if (visible == location) NoteArrival();
  }

  new_line;
  style bold;
  if (visibility_levels == 0) print (name) thedark;
  else
  {   if (visible ~= location) print (The) visible;
      else print (name) visible;
  }
  style roman;

  for (j=1, i=parent(player):j<visibility_levels:j++, i=parent(i))
      if (i has supporter) L__M(##Look,1,i);
                      else L__M(##Look,2,i);

  if (print_player_flag==1) L__M(##Look,3,player);
  new_line;

  if (lookmode<3 && visible==location)
  {   if ((allow_abbrev~=1) || (lookmode==2) || (location hasnt visited))
      {   if (location.describe~=NULL) RunRoutines(location,describe);
          else
          {   if (location.description==0) RunTimeError(11,location);
              else PrintOrRun(location,description);
          }
      }
  }

  if (visibility_levels == 0) Locale(thedark);
  else
  for (j=visibility_levels: j>0: j--)
  {   for (i=player, k=0: k<j: k++) i=parent(i);
      if (i.inside_description~=0)
      {   new_line; PrintOrRun(i,inside_description); }
      Locale(i);
  }

  LookRoutine();
  ScoreArrival();

  action=##Look;
  if (AfterRoutines()==1) rtrue;
];

[ ExamineSub i;
  if (location==thedark) return L__M(##Examine,1);
  i=noun.description;
  if (i==0)
  {   if (noun has container) <<Search noun>>;
      if (noun has switchable) { L__M(##Examine,3,noun); rfalse; }
      return L__M(##Examine,2,noun);
  }
  PrintOrRun(noun, description);
  if (noun has switchable) L__M(##Examine,3,noun);
  if (AfterRoutines()==1) rtrue;
];

[ LookUnderSub;
  if (location==thedark) return L__M(##LookUnder,1);
  L__M(##LookUnder,2);
];

[ SearchSub i f;
  if (location==thedark) return L__M(##Search,1,noun);
  if (ObjectIsUntouchable(noun)) return;
  objectloop (i in noun) if (i hasnt concealed && i hasnt scenery) f=1;
  if (noun has supporter)
  {   if (f==0) return L__M(##Search,2,noun);
      return L__M(##Search,3,noun);
  }
  if (noun hasnt container) return L__M(##Search,4,noun);
  if (noun hasnt transparent && noun hasnt open)
      return L__M(##Search,5,noun);
  if (AfterRoutines()==1) rtrue;

  i=children(noun);
  if (f==0) return L__M(##Search,6,noun);
  L__M(##Search,7,noun);
];

! ----------------------------------------------------------------------------
!   Verbs which change the state of objects without moving them
! ----------------------------------------------------------------------------

[ UnlockSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun hasnt lockable) return L__M(##Unlock,1,noun);
  if (noun hasnt locked)   return L__M(##Unlock,2,noun);
  if (noun.with_key~=second) return L__M(##Unlock,3,second);
  give noun ~locked;
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  L__M(##Unlock,4,noun);
];

[ LockSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun hasnt lockable) return L__M(##Lock,1,noun);
  if (noun has locked)     return L__M(##Lock,2,noun);
  if (noun has open)       return L__M(##Lock,3,noun);
  if (noun.with_key~=second) return L__M(##Lock,4,second);
  give noun locked;
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  L__M(##Lock,5,noun);
];

[ SwitchonSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun hasnt switchable) return L__M(##SwitchOn,1,noun);
  if (noun has on) return L__M(##SwitchOn,2,noun);
  give noun on;
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  L__M(##SwitchOn,3,noun);
];

[ SwitchoffSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun hasnt switchable) return L__M(##SwitchOff,1,noun);
  if (noun hasnt on) return L__M(##SwitchOff,2,noun);
  give noun ~on;
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  L__M(##SwitchOff,3,noun);
];

[ OpenSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun hasnt openable) return L__M(##Open,1,noun);
  if (noun has locked)     return L__M(##Open,2,noun);
  if (noun has open)       return L__M(##Open,3,noun);
  give noun open;
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  if (noun has container && noun hasnt transparent && child(noun)~=0
      && IndirectlyContains(noun,player)==0)
      return L__M(##Open,4,noun);
  L__M(##Open,5,noun);
];

[ CloseSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun hasnt openable) return L__M(##Close,1,noun);
  if (noun hasnt open)     return L__M(##Close,2,noun);
  give noun ~open;
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  L__M(##Close,3,noun);
];

[ DisrobeSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun hasnt worn) return L__M(##Disrobe,1,noun);
  give noun ~worn;
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  L__M(##Disrobe,2,noun);
];

[ WearSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun hasnt clothing)  return L__M(##Wear,1,noun);
  if (parent(noun)~=player) return L__M(##Wear,2,noun);
  if (noun has worn)        return L__M(##Wear,3,noun);
  give noun worn;
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  L__M(##Wear,4,noun);
];

[ EatSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun hasnt edible) return L__M(##Eat,1,noun);
  remove noun;
  if (AfterRoutines()==1) rtrue;
  if (keep_silent==1) rtrue;
  L__M(##Eat,2,noun);
];

! ----------------------------------------------------------------------------
!   Verbs which are really just stubs (anything which happens for these
!   actions must happen in before rules)
! ----------------------------------------------------------------------------

[ YesSub; L__M(##Yes); ];
[ NoSub; L__M(##No); ];
[ BurnSub; L__M(##Burn,1,noun); ];
[ PraySub; L__M(##Pray,1,noun); ];
[ WakeSub; L__M(##Wake,1,noun); ];
[ WakeOtherSub;
  if (ObjectIsUntouchable(noun)) return;
  if (RunLife(noun,##WakeOther)~=0) rfalse;
  L__M(##WakeOther,1,noun);
];
[ ThinkSub; L__M(##Think,1,noun); ];
[ SmellSub; L__M(##Smell,1,noun); ];
[ ListenSub; L__M(##Listen,1,noun); ];
[ TasteSub; L__M(##Taste,1,noun); ];
[ DigSub; L__M(##Dig,1,noun); ];
[ CutSub; L__M(##Cut,1,noun); ];
[ JumpSub; L__M(##Jump,1,noun); ];
[ JumpOverSub; L__M(##JumpOver,1,noun); ];
[ TieSub; L__M(##Tie,1,noun); ];
[ DrinkSub; L__M(##Drink,1,noun); ];
[ FillSub; L__M(##Fill,1,noun); ];
[ SorrySub; L__M(##Sorry,1,noun); ];
[ StrongSub; L__M(##Strong,1,noun); ];
[ MildSub; L__M(##Mild,1,noun); ];
[ SwimSub; L__M(##Swim,1,noun); ];
[ SwingSub; L__M(##Swing,1,noun); ];
[ BlowSub; L__M(##Blow,1,noun); ];
[ RubSub; L__M(##Rub,1,noun); ];
[ SetSub; L__M(##Set,1,noun); ];
[ SetToSub; L__M(##SetTo,1,noun); ];
[ WaveHandsSub; L__M(##WaveHands,1,noun); ];
[ BuySub; L__M(##Buy,1,noun); ];
[ SingSub; L__M(##Sing,1,noun); ];
[ ClimbSub; L__M(##Climb,1,noun); ];
[ SleepSub; L__M(##Sleep,1,noun); ];
[ ConsultSub; L__M(##Consult,1,noun); ];
[ TouchSub;
  if (noun==player) return L__M(##Touch,3,noun);
  if (ObjectIsUntouchable(noun)) return;
  if (noun has animate) return L__M(##Touch,1,noun);
  L__M(##Touch,2,noun); ];
[ WaveSub;
  if (parent(noun)~=player) return L__M(##Wave,1,noun);
  L__M(##Wave,2,noun); ];
[ PullSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun has static)   return L__M(##Pull,1,noun);
  if (noun has scenery)  return L__M(##Pull,2,noun);
  if (noun has animate)  return L__M(##Pull,4,noun);
  L__M(##Pull,3,noun);
];
[ PushSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun has static)   return L__M(##Push,1,noun);
  if (noun has scenery)  return L__M(##Push,2,noun);
  if (noun has animate)  return L__M(##Pull,4,noun);
  L__M(##Push,3,noun);
];
[ TurnSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun has static)   return L__M(##Turn,1,noun);
  if (noun has scenery)  return L__M(##Turn,2,noun);
  if (noun has animate)  return L__M(##Pull,4,noun);
  L__M(##Turn,3,noun);
];

[ WaitSub;
  if (AfterRoutines()==1) rtrue;
  L__M(##Wait,1,noun);
];

[ PushDirSub; L__M(##PushDir,1,noun); ];
[ AllowPushDir i;
  if (parent(second)~=compass) return L__M(##PushDir,2,noun);
  if (second==u_obj or d_obj)  return L__M(##PushDir,3,noun);
  AfterRoutines(); i=noun; move i to player;
  <Go second>;
  if (location==thedark) move i to real_location;
  else move i to location;
];

[ SqueezeSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun has animate) return L__M(##Squeeze,1,noun);
  L__M(##Squeeze,2,noun);
];

[ ThrowAtSub;
  if (ObjectIsUntouchable(noun)) return;
  if (second>1)
  {   action=##ThrownAt;
      if (RunRoutines(second,before)~=0) { action=##ThrowAt; rtrue; }
      action=##ThrowAt;
  }
  if (second hasnt animate) return L__M(##ThrowAt,1);
  if (RunLife(second,##ThrowAt)~=0) rfalse;
  L__M(##ThrowAt,2,noun);
];

[ AttackSub;
  if (ObjectIsUntouchable(noun)) return;
  if (noun has animate && RunLife(noun,##Attack)~=0) rfalse;
  L__M(##Attack,1,noun); ];

[ KissSub;
  if (ObjectIsUntouchable(noun)) return;
  if (RunLife(noun,##Kiss)~=0) rfalse;
  if (noun==player) return L__M(##Touch,3,noun);
  L__M(##Kiss,1,noun);
];

[ AnswerSub;
  if (RunLife(second,##Answer)~=0) rfalse;
  L__M(##Answer,1,noun);
];  

[ TellSub;
  if (noun==player) return L__M(##Tell,1,noun);
  if (RunLife(noun,##Tell)~=0) rfalse;
  L__M(##Tell,2,noun);
];  
  
[ AskSub;
  if (RunLife(noun,##Ask)~=0) rfalse;
  L__M(##Ask,1,noun);
];  

[ AskForSub;
  if (noun==player) <<Inv>>;
  L__M(##Order,1,noun);
];

! ----------------------------------------------------------------------------
!   Debugging verbs
! ----------------------------------------------------------------------------

#IFDEF DEBUG;
[ TraceOnSub; parser_trace=1; "[Trace on.]"; ];
[ TraceLevelSub; parser_trace=noun;
  print "[Parser tracing set to level ", parser_trace, ".]^"; ];
[ TraceOffSub; parser_trace=0; "Trace off."; ];
[ RoutinesOnSub;  debug_flag=debug_flag | 1; "[Message listing on.]"; ];
[ RoutinesOffSub; debug_flag=debug_flag & 6; "[Message listing off.]"; ];
[ ActionsOnSub;  debug_flag=debug_flag | 2; "[Action listing on.]"; ];
[ ActionsOffSub; debug_flag=debug_flag & 5; "[Action listing off.]"; ];
[ TimersOnSub;  debug_flag=debug_flag | 4; "[Timers listing on.]"; ];
[ TimersOffSub; debug_flag=debug_flag & 3; "[Timers listing off.]"; ];
[ CommandsOnSub;
  @output_stream 4; xcommsdir=1; "[Command recording on.]"; ];
[ CommandsOffSub;
  if (xcommsdir==1) @output_stream -4;
  xcommsdir=0;
  "[Command recording off.]"; ];
[ CommandsReadSub;
  @input_stream 1; xcommsdir=2; "[Replaying commands.]"; ];
[ PredictableSub i; i=random(-100);
  "[Random number generator now predictable.]"; ];
[ XTestMove obj dest;
  if ((obj<=InformLibrary) || (obj == LibraryMessages) || (obj in 1))
     "[Can't move ", (name) obj, ": it's a system object.]";
  while (dest ~= 0)
  {   if (dest == obj)
          "[Can't move ", (name) obj, ": it would contain itself.]";
      dest = parent(dest);
  }
  rfalse;
];
[ XPurloinSub;
  if (XTestMove(noun,player)) return;
  move noun to player; give noun moved ~concealed;
  "[Purloined.]"; ];
[ XAbstractSub;
  if (XTestMove(noun,second)) return;
  move noun to second; "[Abstracted.]"; ];
[ XObj obj f;
  if (parent(obj) == 0) print (name) obj; else print (a) obj;
  print " (", obj, ") ";
  if (f==1 && parent(obj) ~= 0)
      print "(in ", (name) parent(obj), " ", parent(obj), ")";
  new_line;
  if (child(obj)==0) rtrue;
  if (obj == Class)
      WriteListFrom(child(obj),
      NOARTICLE_BIT + INDENT_BIT + NEWLINE_BIT + ALWAYS_BIT, 1);
  else
      WriteListFrom(child(obj),
      FULLINV_BIT + INDENT_BIT + NEWLINE_BIT + ALWAYS_BIT, 1);
];
[ XTreeSub i;
  if (noun==0)
  {   objectloop(i) if (parent(i)==0) XObj(i);
  }
  else XObj(noun,1);
];
[ GotoSub;
  if (~~(noun ofclass Object) || (parent(noun)~=0)) "[Not a safe place.]";
  PlayerTo(noun);
];
[ GonearSub x; x=noun; while (parent(x)~=0) x=parent(x); PlayerTo(x); ];
[ Print_ScL obj; print_ret ++x_scope_count, ": ", (a) obj, " (", obj, ")"; ];
[ ScopeSub; x_scope_count=0; LoopOverScope(#r$Print_ScL, noun);
  if (x_scope_count==0) "Nothing is in scope.";
];
#ENDIF;

! ----------------------------------------------------------------------------
!   Finally: the mechanism for library text (the text is in the language defn)
! ----------------------------------------------------------------------------

[ L__M act n x1 s;
  s=sw__var; sw__var=act; if (n==0) n=1;
  L___M(n,x1);
  sw__var=s;
];

[ L___M n x1 s;
  s=action;
  lm_n=n; lm_o=x1;
  action=sw__var;
  if (RunRoutines(LibraryMessages,before)~=0) { action=s; rfalse; }
  action=s;

  LanguageLM(n, x1);
];

! ----------------------------------------------------------------------------
