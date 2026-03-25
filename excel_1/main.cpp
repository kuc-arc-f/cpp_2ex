#include <xlsxwriter.h>
#include <string>

int main() {
    // ワークブック・シート作成
    lxw_workbook  *workbook  = workbook_new("output.xlsx");
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, "Sheet1");

    // ヘッダー書き込み（行, 列, 値）
    worksheet_write_string(worksheet, 0, 0, "名前",   NULL);
    worksheet_write_string(worksheet, 0, 1, "年齢",   NULL);
    worksheet_write_string(worksheet, 0, 2, "スコア", NULL);

    // データ書き込み
    worksheet_write_string(worksheet, 1, 0, "hoge", NULL);
    worksheet_write_number(worksheet, 1, 1, 28,        NULL);
    worksheet_write_number(worksheet, 1, 2, 95.5,      NULL);

    worksheet_write_string(worksheet, 2, 0, "fuga", NULL);
    worksheet_write_number(worksheet, 2, 1, 34,        NULL);
    worksheet_write_number(worksheet, 2, 2, 87.0,      NULL);

    // ファイル保存
    workbook_close(workbook);
    return 0;
}