/********************************************************************\
 * gnc-filepath-utils.h -- file path resolution utilities           *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
\********************************************************************/

/**
 * @file gnc-filepath-utils.h
 * @brief File path resolution utility functions
 * @author Copyright (c) 1998-2004 Linas Vepstas <linas@linas.org>
 * @author Copyright (c) 2000 Dave Peticolas
 */

#ifndef GNC_FILEPATH_UTILS_H
#define GNC_FILEPATH_UTILS_H

/** The gnc_resolve_file_path() routine is a utility that will accept
 *    a fragmentary filename as input, and resolve it into a fully
 *    qualified path in the file system, i.e. a path that begins with
 *    a leading slash.  First, the current working directory is
 *    searched for the file.  Next, the directory $HOME/.gnucash/data,
 *    and finally, a list of other (configurable) paths.  If the file
 *    is not found, then the path $HOME/.gnucash/data is used.  If
 *    $HOME is not defined, then the current working directory is
 *    used.
 */
gchar *gnc_resolve_file_path (const gchar *filefrag);

const gchar *gnc_dotgnucash_dir (void);
gchar *gnc_build_dotgnucash_path (const gchar *filename);
gchar *gnc_build_book_path (const gchar *filename);
gchar *gnc_build_translog_path (const gchar *filename);
gchar *gnc_build_data_path (const gchar *filename);
gchar *gnc_build_report_path (const gchar *filename);
gchar *gnc_build_stdreports_path (const gchar *filename);

/** Given a pixmap/pixbuf file name, find the file in the pixmap
 *  directory associated with this application.  This routine will
 *  display an error message if it can't find the file.
 *
 *  @param name The name of the file to be found.
 *
 *  @return the full path name of the file, or NULL of the file can't
 *  be found.
 *
 *  @note It is the caller's responsibility to free the returned string.
 */
gchar *gnc_filepath_locate_pixmap (const gchar *name);


/** Given a file name, find the file in the directories associated
 *  with this application.  This routine will display an error message
 *  if it can't find the file.
 *
 *  @param name The name of the file to be found.
 *
 *  @return the full path name of the file, or NULL of the file can't
 *  be found.
 *
 *  @note It is the caller's responsibility to free the returned string.
 */
gchar *gnc_filepath_locate_data_file (const gchar *name);


/** Given a ui file name, find the file in the ui directory associated
 *  with this application.  This routine will display an error message
 *  if it can't find the file.
 *
 *  @param name The name of the file to be found.
 *
 *  @return the full path name of the file, or NULL of the file can't
 *  be found.
 *
 *  @note It is the caller's responsibility to free the returned string.
 */
gchar *gnc_filepath_locate_ui_file (const gchar *name);


/** Given a documentation file name, find the file in the doc directory
 *  associated with this application.  This routine will display an error
 *  message if it can't find the file.
 *
 *  @param name The name of the file to be found.
 *
 *  @return the full path name of the file, or NULL of the file can't
 *  be found.
 *
 *  @note It is the caller's responsibility to free the returned string.
 */
gchar *gnc_filepath_locate_doc_file (const gchar *name);

#endif /* GNC_FILEPATH_UTILS_H */
