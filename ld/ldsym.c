/* All symbol handling for the linker
   Copyright (C) 1991 Free Software Foundation, Inc.
   Written by Steve Chamberlain steve@cygnus.com
 
This file is part of GLD, the Gnu Linker.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*
 *  $Id$ 
 */

/* 
   We keep a hash table of global symbols. Each entry in a hash table
   is called an ldsym_type. Each has three chains; a pointer to a
   chain of definitions for the symbol (hopefully one long), a pointer
   to a chain of references to the symbol, and a pointer to a chain of
   common symbols. Each pointer points into the canonical symbol table
   provided by bfd, each one of which points to an asymbol. Duringing
   linkage, the linker uses the udata field to point to the next entry
   in a canonical table....


   ld_sym
			|          |
   +----------+		+----------+
   | defs     |      a canonical symbol table
   +----------+         +----------+
   | refs     | ----->  | one entry|  -----> asymbol
   +----------+		+----------+	   |   	     |
   | coms     |		|          |	   +---------+
   +----------+		+----------+	   | udata   |-----> another canonical symbol
					   +---------+				     



   It is very simple to make all the symbol pointers point to the same
   definition - just run down the chain and make the asymbols pointers
   within the canonical table point to the asymbol attacthed to the
   definition of the symbol.

*/

#include "bfd.h"
#include "sysdep.h"

#include "ld.h"
#include "ldsym.h"
#include "ldmisc.h"
#include "ldlang.h"
/* IMPORT */

extern bfd *output_bfd;
extern strip_symbols_type strip_symbols;
extern discard_locals_type discard_locals;
/* Head and tail of global symbol table chronological list */

ldsym_type *symbol_head = (ldsym_type *)NULL;
ldsym_type **symbol_tail_ptr = &symbol_head;

extern ld_config_type config;

/*
  incremented for each symbol in the ldsym_type table
  no matter what flavour it is 
*/
unsigned int global_symbol_count;

/* IMPORTS */

extern boolean option_longmap ;

/* LOCALS */
#define	TABSIZE	1009
static ldsym_type *global_symbol_hash_table[TABSIZE];

/* Compute the hash code for symbol name KEY.  */
static 
#ifdef __GNUC__
__inline
#endif

int
DEFUN(hash_string,(key),
      CONST char *key)
{
  register CONST char *cp;
  register int k;

  cp = key;
  k = 0;
  while (*cp)
    k = (((k << 1) + (k >> 14)) ^ (*cp++)) & 0x3fff;

  return k;
}

static
#ifdef __GNUC__
__inline
#endif ldsym_type *bp;
ldsym_type *
DEFUN(search,(key,hashval) ,
      CONST char *key AND
      int hashval)
{
  ldsym_type *bp;				   
  for (bp = global_symbol_hash_table[hashval]; bp; bp = bp->link)
    if (! strcmp (key, bp->name)) {
      if (bp->flags & SYM_INDIRECT) {	
	/* Use the symbol we're aliased to instead */
	return (ldsym_type *)(bp->sdefs_chain);
      }
      return bp;
    }
  return 0;
}


/* Get the symbol table entry for the global symbol named KEY.
   Create one if there is none.  */
ldsym_type *
DEFUN(ldsym_get,(key),
      CONST     char *key)
{
  register int hashval;
  register ldsym_type *bp;

  /* Determine the proper bucket.  */

  hashval = hash_string (key) % TABSIZE;

  /* Search the bucket.  */
  bp = search(key, hashval);
  if(bp) {
    return bp;
  }

  /* Nothing was found; create a new symbol table entry.  */

  bp = (ldsym_type *) ldmalloc ((bfd_size_type)(sizeof (ldsym_type)));
  bp->srefs_chain = (asymbol **)NULL;
  bp->sdefs_chain = (asymbol **)NULL;
  bp->scoms_chain = (asymbol **)NULL;
  bp->name = buystring(key);
  bp->flags = 0;
  /* Add the entry to the bucket.  */

  bp->link = global_symbol_hash_table[hashval];
  global_symbol_hash_table[hashval] = bp;

  /* Keep the chronological list up to date too */
  *symbol_tail_ptr = bp;
  symbol_tail_ptr = &bp->next;
  bp->next = 0;
  global_symbol_count++;

  return bp;
}

/* Like `ldsym_get' but return 0 if the symbol is not already known.  */

ldsym_type *
DEFUN(ldsym_get_soft,(key),
      CONST char *key)
{
  register int hashval;
  /* Determine which bucket.  */

  hashval = hash_string (key) % TABSIZE;

  /* Search the bucket.  */
  return search(key, hashval);
}





static void
list_file_locals (entry)
lang_input_statement_type *entry;
{
  asymbol **q;
  fprintf (config.map_file, "\nLocal symbols of ");
  minfo("%I", entry);
  fprintf (config.map_file, ":\n\n");
  if (entry->asymbols) {
    for (q = entry->asymbols; *q; q++) 
      {
	asymbol *p = *q;
	/* If this is a definition,
	   update it if necessary by this file's start address.  */
	if (p->flags & BSF_LOCAL)
	 info("  %V %s\n",p->value, p->name);
      }
  }
}


static void
DEFUN(print_file_stuff,(f),
      lang_input_statement_type *f)
{
  fprintf (config.map_file,"  %s\n", f->filename);
  if (f->just_syms_flag) 
  {
    fprintf (config.map_file, " symbols only\n");
  }
  else 
  {
    asection *s;
    if (true || option_longmap) {
	for (s = f->the_bfd->sections;
	     s != (asection *)NULL;
	     s = s->next) {
	    print_address(s->output_offset);
	    if (s->reloc_done)
	    {
	      fprintf (config.map_file, " %08x 2**%2ud %s\n",
		      (unsigned)bfd_get_section_size_after_reloc(s),
		      s->alignment_power, s->name);
	    }
	    
	    else 
	    {
	      fprintf (config.map_file, " %08x 2**%2ud %s\n",
		      (unsigned)bfd_get_section_size_before_reloc(s),
		      s->alignment_power, s->name);
	    }
	    

	      
	  }
      }
    else {	      
	for (s = f->the_bfd->sections;
	     s != (asection *)NULL;
	     s = s->next) {
	    fprintf(config.map_file, "%s ", s->name);
	    print_address(s->output_offset);
	    fprintf(config.map_file, "(%x)", (unsigned)bfd_get_section_size_after_reloc(s));
	  }
	fprintf(config.map_file, "hex \n");
      }
  }
  fprintf (config.map_file, "\n");
}

void
ldsym_print_symbol_table ()
{
  fprintf (config.map_file, "**FILES**\n\n");

  lang_for_each_file(print_file_stuff);

  fprintf(config.map_file, "**GLOBAL SYMBOLS**\n\n");
  fprintf(config.map_file, "offset    section    offset   symbol\n");
  {
    register ldsym_type *sp;

    for (sp = symbol_head; sp; sp = sp->next)
      {
	if (sp->flags & SYM_INDIRECT) {
	  fprintf(config.map_file,"indirect %s to %s\n",
		  sp->name, (((ldsym_type *)(sp->sdefs_chain))->name));
	}
	else {
	  if (sp->sdefs_chain) 
	    {
	      asymbol *defsym = *(sp->sdefs_chain);
	      asection *defsec = bfd_get_section(defsym);
	      print_address(defsym->value);
	      if (defsec)
		{
		  fprintf(config.map_file, "  %-10s",
			 bfd_section_name(output_bfd,
					  defsec));
		  print_space();
		  print_address(defsym->value+defsec->vma);

		}
	      else 
		{
		  fprintf(config.map_file, "         .......");
		}

	    }	


	  if (sp->scoms_chain) {
	    fprintf(config.map_file, "common               ");
	    print_address((*(sp->scoms_chain))->value);
	    fprintf(config.map_file, " %s ",sp->name);
	  }
	  else if (sp->sdefs_chain) {
	    fprintf(config.map_file, " %s ",sp->name);
	  }
	  else {
	    fprintf(config.map_file, "undefined                     ");
	    fprintf(config.map_file, "%s ",sp->name);

	  }
	}
	print_nl();

      }
  }
  if (option_longmap) {
    lang_for_each_file(list_file_locals);
  }
}

extern lang_output_section_statement_type *create_object_symbols;
extern char lprefix;
static asymbol **
write_file_locals(output_buffer)
asymbol **output_buffer;
{
LANG_FOR_EACH_INPUT_STATEMENT(entry)
    {
      /* Run trough the symbols and work out what to do with them */
      unsigned int i;

      /* Add one for the filename symbol if needed */
      if (create_object_symbols 
	  != (lang_output_section_statement_type *)NULL) {
	asection *s;
	for (s = entry->the_bfd->sections;
	     s != (asection *)NULL;
	     s = s->next) {
	  if (s->output_section == create_object_symbols->bfd_section) {
	    /* Add symbol to this section */
	    asymbol * newsym  =
	      (asymbol *)bfd_make_empty_symbol(entry->the_bfd);
	    newsym->name = entry->local_sym_name;
	    /* The symbol belongs to the output file's text section */

	    /* The value is the start of this section in the output file*/
	    newsym->value  = 0;
	    newsym->flags = BSF_LOCAL;
	    newsym->section = s;
	    *output_buffer++ = newsym;
	    break;
	  }
	}
      }
      for (i = 0; i < entry->symbol_count; i++) 
	{
	  asymbol *p = entry->asymbols[i];
	  /* FIXME, temporary hack, since not all of ld knows about the new abs section convention */

	  if (p->section == 0)
	    p->section = &bfd_abs_section;
	  if (flag_is_global(p->flags) )
	    {
	      /* We are only interested in outputting 
		 globals at this stage in special circumstances */
	      if (p->the_bfd == entry->the_bfd 
		  && flag_is_not_at_end(p->flags)) {
		/* And this is one of them */
		*(output_buffer++) = p;
		p->flags |= BSF_KEEP;
	      }
	    }
	  else {
	    if (flag_is_ordinary_local(p->flags)) 
	      {
		if (discard_locals == DISCARD_ALL)
		  {  }
		else if (discard_locals == DISCARD_L &&
			 (p->name[0] == lprefix)) 
		  {  }
		else if (p->flags ==  BSF_WARNING) 
		  {  }
		else 
		  { *output_buffer++ = p; }
	      }
	    else if (flag_is_debugger(p->flags)) 
	      {
		/* Only keep the debugger symbols if no stripping required */
		if (strip_symbols == STRIP_NONE) {
		  *output_buffer++ = p;
		}
	      }
	    else if (p->section == &bfd_und_section)
	      { /* This must be global */
	      }
	    else if (p->section == &bfd_com_section) {
	   /* And so must this */
	    } 
	    else if (p->flags & BSF_CTOR) {
	      /* Throw it away */
	    }
else
	      {
		FAIL();
	      }
	  }
	}


    }
  return output_buffer;
}


static asymbol **
write_file_globals(symbol_table)
asymbol **symbol_table;
{
  FOR_EACH_LDSYM(sp)
    {
      if ((sp->flags & SYM_INDIRECT) == 0 && sp->sdefs_chain != (asymbol **)NULL) {
	asymbol *bufp = (*(sp->sdefs_chain));

	if ((bufp->flags & BSF_KEEP) ==0) {
	  ASSERT(bufp != (asymbol *)NULL);

	  bufp->name = sp->name;

	  if (sp->scoms_chain != (asymbol **)NULL)	

	    {
	      /* 
		 defined as common but not allocated, this happens
		 only with -r and not -d, write out a common
		 definition
		 */
	      bufp = *(sp->scoms_chain);
	    }
	  *symbol_table++ = bufp;
	}
      }
      else if (sp->scoms_chain != (asymbol **)NULL) {
	/* This symbol is a common - just output */
	asymbol *bufp = (*(sp->scoms_chain));
	*symbol_table++ = bufp;
      }
      else if (sp->srefs_chain != (asymbol **)NULL) {
	/* This symbol is undefined but has a reference */
	asymbol *bufp = (*(sp->srefs_chain));
	*symbol_table++ = bufp;
      }
      else {
	/*
	   This symbol has neither defs nor refs, it must have come
	   from the command line, since noone has used it it has no
	   data attatched, so we'll ignore it 
	   */
      }
    }
  return symbol_table;
}



void
ldsym_write()
{
  if (strip_symbols != STRIP_ALL) {
    /* We know the maximum size of the symbol table -
       it's the size of all the global symbols ever seen +
       the size of all the symbols from all the files +
       the number of files (for the per file symbols)
       +1 (for the null at the end)
       */
    extern unsigned int total_files_seen;
    extern unsigned int total_symbols_seen;

    asymbol **  symbol_table =  (asymbol **) 
      ldmalloc ((bfd_size_type)(global_symbol_count +
			 total_files_seen +
			 total_symbols_seen + 1) *     sizeof (asymbol *));
    asymbol ** tablep = write_file_locals(symbol_table);

    tablep = write_file_globals(tablep);

    *tablep =  (asymbol *)NULL;
    bfd_set_symtab(output_bfd, symbol_table, (unsigned)( tablep - symbol_table));
  }
}

/*
return true if the supplied symbol name is not in the 
linker symbol table
*/
boolean 
DEFUN(ldsym_undefined,(sym),
      CONST char *sym)
{
  ldsym_type *from_table = ldsym_get_soft(sym);
  if (from_table != (ldsym_type *)NULL) {
    if (from_table->sdefs_chain != (asymbol **)NULL) return false;
  }
  return true;
}
