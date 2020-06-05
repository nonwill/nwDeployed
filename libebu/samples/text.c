/*                                                            -*- C -*-
 * Copyright (c) 1999-2006  Motoyuki Kasahara
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * 使用方法:
 *     text <book-path> <subbook-index> <number>
 * 例:
 *     text /cdrom 0 10
 * 説明:
 *     <book-path> で指定した CD-ROM 書籍から特定の副本を選び、本文
 *     の先頭から <number> 個分の単語の説明を出力します。
 *
 *     <subbook-index> には、検索対象の副本のインデックスを指定しま
 *     す。インデックスは、書籍の最初の副本から順に 0、1、2 ... に
 *     なります。
 */
#include <stdio.h>
#include <stdlib.h>

#include <eb/eb.h>
#include <eb/error.h>
#include <eb/text.h>

#define MAXLEN_TEXT 1023

int
main(int argc, char *argv[])
{
    EB_Error_Code error_code;
    EB_Book book;
    EB_Subbook_Code subbook_list[EB_MAX_SUBBOOKS];
    int subbook_count;
    int subbook_index;
    EB_Position text_position;
    char text[MAXLEN_TEXT + 1];
    ssize_t text_length;
    int text_count;
    int i;

    /* コマンド行引数をチェック。*/
    if (argc != 4) {
        fprintf(stderr, "Usage: %s book-path subbook-index number\n",
            argv[0]);
        exit(1);
    }
    text_count = atoi(argv[3]);

    /* EB ライブラリと `book' を初期化。*/
    eb_initialize_library();
    eb_initialize_book(&book);

    /* 書籍を `book' に結び付ける。*/
    error_code = eb_bind(&book, argv[1]);
    if (error_code != EB_SUCCESS) {
        fprintf(stderr, "%s: failed to bind the book, %s: %s\n",
            argv[0], eb_error_message(error_code), argv[1]);
        goto die;
    }

    /* 副本の一覧を取得。*/
    error_code = eb_subbook_list(&book, subbook_list, &subbook_count);
    if (error_code != EB_SUCCESS) {
        fprintf(stderr, "%s: failed to get the subbbook list, %s\n",
            argv[0], eb_error_message(error_code));
        goto die;
    }

    /* 副本のインデックスを取得。*/
    subbook_index = atoi(argv[2]);

    /*「現在の副本 (current subbook)」を設定。*/
    error_code = eb_set_subbook(&book, subbook_list[subbook_index]);
    if (error_code != EB_SUCCESS) {
        fprintf(stderr, "%s: failed to set the current subbook, %s\n",
            argv[0], eb_error_message(error_code));
        goto die;
    }

    /* テキストの開始位置を取得。*/
    error_code = eb_text(&book, &text_position);
    if (error_code != EB_SUCCESS) {
        fprintf(stderr, "%s: failed to get text information, %s\n",
            argv[0], eb_error_message(error_code));
        goto die;
    }

    /* テキストをシーク。*/
    error_code = eb_seek_text(&book, &text_position);
    if (error_code != EB_SUCCESS) {
        fprintf(stderr, "%s: failed to seek text, %s\n",
            argv[0], eb_error_message(error_code));
        goto die;
    }

    i = 0;
    while (i < text_count) {
        /* テキストを取得。*/
	error_code = eb_read_text(&book, NULL, NULL, NULL, MAXLEN_TEXT,
	    text, &text_length);
	if (error_code != EB_SUCCESS) {
            fprintf(stderr, "%s: failed to read text, %s\n",
                argv[0], eb_error_message(error_code));
            goto die;
        }
	fputs(text, stdout);

        if (!eb_is_text_stopped(&book))
	    continue;

	fputs("\n----------------------------------------\n", stdout);

        /* 次の単語の説明へ移動。*/
	error_code = eb_forward_text(&book, NULL);
	if (error_code == EB_ERR_END_OF_CONTENT)
	    fputs("\n[END]\n", stdout);
	else if (error_code != EB_SUCCESS) {
	    fprintf(stderr, "%s: failed to read text, %s\n",
		argv[0], eb_error_message(error_code));
	    goto die;
	}
	i++;
    }
        
    /* 書籍と EB ライブラリの利用を終了。*/
    eb_finalize_book(&book);
    eb_finalize_library();
    exit(0);

    /* エラー発生で終了するときの処理。*/
  die:
    eb_finalize_book(&book);
    eb_finalize_library();
    exit(1);
}
