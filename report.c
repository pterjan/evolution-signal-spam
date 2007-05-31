/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 *  Copyright 2007 Pascal Terjan <pterjan@linuxfr.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of version 2 of the GNU General Public
 *  License as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <glib/gbase64.h>
#include <glib/gmem.h>

gboolean send_report(char *message, const char *user, const char *password);

gboolean send_report(char *message, const char *user, const char *password)
{
  CURL *curl;
  CURLcode res;
  char *post, *msg, *userpwd;

  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();
  if (!curl) {
    free(message);
    return 0;
  }

  curl_easy_setopt(curl, CURLOPT_URL, "https://www.signal-spam.fr/api/signaler");
  
  msg = g_base64_encode(message, strlen(message));
  free(message);
  post = (char*)malloc(strlen("message=")+strlen(msg)+1);
  sprintf(post, "message=%s", msg);
  g_free(msg);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);

  userpwd = (char*)malloc(strlen(user)+strlen(password)+2);

  sprintf(userpwd, "%s:%s", user, password);
  curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);

  res = curl_easy_perform(curl);

  free(post);
  free(userpwd);

  curl_easy_cleanup(curl);
  return 0;
}
