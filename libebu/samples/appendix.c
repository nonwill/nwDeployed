/*                                                            -*- C -*-
 * Copyright (c) 2003-2006  Motoyuki Kasahara
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
 *     font <appendix-path> <subbook-index>
 * 例:
 *     font /cdrom 0
 * 説明:
 *     <appendix-path> で指定した appendix から特定の副本を選び、そ
 *     の副本が定義している半角外字の代替文字列をすべて表示します。
 *
 *     その appendix が、半角外字の代替文字列を定義していないと、エ
 *     ラーになります。
 *
 *     <subbook-index> には、操作対象の副本のインデックスを指定しま
 *     す。インデックスは、書籍の最初の副本から順に 0、1、2 ... に
 *     なります。
 */
#include <stdio.h>
#include <stdlib.h>

#include <eb/eb.h>
#include <eb/error.h>
#include <eb/appendix.h>

int
main(int argc, char *argv[])
{
    EB_Error_Code error_code;
    EB_Appendix app;
    EB_Subbook_Code subbook_list[EB_MAX_SUBBOOKS];
    int subbook_count;
    int subbook_index;
    int alt_start;
    char text[EB_MAX_ALTERNATION_TEXT_LENGTH + 1];
    int i;

    /* コマンド行引数をチェック。*/
    if (argc != 3) {
        fprintf(stderr, "Usage: %s appendix-path subbook-index\n",
            argv[0]);
        exit(1);
    }

    /* EB ライブラリと `app' を初期化。*/
    eb_initialize_library();
    eb_initialize_appendix(&app);

    /* appendix を `app' に結び付ける。*/
    error_code = eb_bind_appendix(&app, argv[1]);
    if (error_code != EB_SUCCESS) {
        fprintf(stderr, "%s: failed to bind the app, %s: %s\n",
            argv[0], eb_error_message(error_code), argv[1]);
        goto die;
    }

    /* 副本の一覧を取得。*/
    error_code = eb_appendix_subbook_list(&app, subbook_list,
	&subbook_count);
    if (error_code != EB_SUCCESS) {
        fprintf(stderr, "%s: failed to get the subbook list, %s\n",
            argv[0], eb_error_message(error_code));
        goto die;
    }

    /* 副本のインデックスを取得。*/
    subbook_index = atoi(argv[2]);

    /*「現在の副本 (current subbook)」を設定。*/
    if (eb_set_appendix_subbook(&app, subbook_list[subbook_index])
	< 0) {
        fprintf(stderr, "%s: failed to set the current subbook, %s\n",
            argv[0], eb_error_message(error_code));
        goto die;
    }

    /* 外字の開始位置を取得。*/
    error_code = eb_narrow_alt_start(&app, &alt_start);
    if (error_code != EB_SUCCESS) {
        fprintf(stderr, "%s: failed to get font information, %s\n",
            argv[0], eb_error_message(error_code));
        goto die;
    }

    i = alt_start;
    for (;;) {
        /* 外字の代替文字列を取得。*/
	error_code = eb_narrow_alt_character_text(&app, i, text);
	if (error_code != EB_SUCCESS) {
            fprintf(stderr, "%s: failed to get font data, %s\n",
                argv[0], eb_error_message(error_code));
            goto die;
        }

	/* 取得した代替文字列を出力。*/
	printf("%04x: %s\n", i, text);

        /* 外字の文字番号を一つ進める。*/
	error_code = eb_forward_narrow_alt_character(&app, 1, &i);
	if (error_code != EB_SUCCESS)
	    break;
    }
        
    /* appendix と EB ライブラリの利用を終了。*/
    eb_finalize_appendix(&app);
    eb_finalize_library();
    exit(0);

    /* エラー発生で終了するときの処理。*/
  die:
    eb_finalize_appendix(&app);
    eb_finalize_library();
    exit(1);
}
