import pandas as pd
import sys

def process_data(df):
    """
    Обрабатывает DataFrame, объединяя парные строки 'par' и 'seq'
    """
    # Группируем по ключевым столбцам (кроме parameter и временных)
    grouped = df.groupby(['executable', 'cores', 'attempt'])
    
    result_dfs = []
    
    for (executable, cores, attempt), group in grouped:
        # Проверяем условия для объединения
        if len(group) == 2:
            par_row = group[group['parameter'] == 'par']
            seq_row = group[group['parameter'] == 'seq']
            
            if len(par_row) == 1 and len(seq_row) == 1:
                # Создаем объединенную строку
                new_row = pd.DataFrame([[
                    executable,
                    'par+seq',
                    cores,
                    attempt,
                    par_row['parallel_time'].values[0],
                    seq_row['sequential_time'].values[0]
                ]], columns=df.columns)
                
                result_dfs.append(new_row)
                continue
        
        # Если не объединяли - добавляем исходные строки
        result_dfs.append(group)
    
    return pd.concat(result_dfs, ignore_index=True)

def csv_to_excel(input_csv, output_excel):
    """
    Конвертирует CSV-файл в Excel-файл с обработкой данных
    """
    # Чтение CSV с сохранением строкового формата
    df = pd.read_csv(
        input_csv,
        dtype=str,
        keep_default_na=False
    )
    
    # # Проверка наличия ожидаемых столбцов
    # expected_columns = {"executable", "parameter", "cores", "attempt", "parallel_time", "sequental_time"}
    # if set(df.columns) != expected_columns:
    #     print("Ошибка: Столбцы в файле не соответствуют ожидаемым!")
    #     print(f"Найдены: {list(df.columns)}")
    #     print(f"Ожидались: {list(expected_columns)}")
    #     return
    
    # Обработка данных
    processed_df = process_data(df)
    
    # Сохранение в Excel
    processed_df.to_excel(output_excel, index=False, engine='openpyxl')
    print(f"Файл успешно конвертирован: {output_excel}")

if __name__ == "__main__":
    # Обработка аргументов командной строки
    if len(sys.argv) != 3:
        print("Использование: python csv_to_excel.py <input.csv> <output.xlsx>")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    # Проверка расширений файлов
    if not input_file.endswith('.csv'):
        print("Предупреждение: Входной файл должен иметь расширение .csv")
    
    if not output_file.endswith('.xlsx'):
        print("Рекомендуемое расширение для выходного файла - .xlsx")
    
    csv_to_excel(input_file, output_file)
