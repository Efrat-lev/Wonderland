
## Level 2 - 

**מטרה:** עקיפת מנגנון האימות של השלב השני בתוכנית על ידי ניתוח אלגוריתם ההצפנה.

כדי להבין מול מה אני מתמודדת, התחלתי בהרצת הקובץ `Wonderland.exe` בשורת הפקודה. כשהזנתי את השלב `2`, קיבלתי את הפלט הבא שמבקש סיסמה:

```text
You know what? That was too easy. *Now* tell me the second password.
(Please enter the password)

```

הזנת סיסמה אקראית הובילה להדפסת `Wrong password!`.

כדי להבין איך הקלט נבדק, פתחתי את הקובץ ב-IDA. התחלתי את החיפוש בחלון ה-Strings ואיתרתי את מחרוזת הפתיחה של השלב. לחיצה כפולה הובילה אותי ל-Data Segment, ומשם השתמשתי בהפניות (Xrefs) כדי לקפוץ אל הפונקציה שמשתמשת במחרוזת הזו - פונקציית האימות `sub_401330`.

![Strings View](images/l2_strings.png)

בתחילת הפונקציה, זיהיתי קריאה שקולטת נתונים מהמשתמש ושומרת אותם לתוך משתנה מקומי (Buffer). מיד לאחר מכן, מתחילה לולאת העיבוד המרכזית:

התבוננות בלולאה חושפת את מנגנון ההצפנה. התוכנית לא מעבדת את הקלט תו-אחר-תו. במקום זאת, ההוראה `mov ecx, dword ptr [ebp+eax+Buffer]` שולפת בלוקים של 4 בתים (DWORD) מתוך ה-Buffer אל האוגר `ecx`.
לאחר מכן, מופעלת הפקודה: `xor ecx, 41524241h`.

![IDA Graph View](images/l2_ida_graph.png)

מכיוון שארכיטקטורת x86 שומרת נתונים בזיכרון בשיטת Little Endian, הערך הקבוע `41524241h` נקרא מהסוף להתחלה כ-`41 42 52 41`. תרגום ערכי ההקסדצימל האלו לתווי ASCII חושף את מפתח ההצפנה המחזורי שלנו: **"ABRA"**.

בסיום לולאת ה-XOR, הפונקציה קוראת ל-`strncmp` כדי להשוות את ה-Buffer המעובד שלנו מול מחרוזת יעד קבועה השמורה בזיכרון: `"into the rabbit hole"`.

**פסאודו-קוד של הלוגיקה:**

<div dir="ltr" align="left">

```text
key = "ABRA"
for (i = 0; i < len(input); i+=4):
    input[i:i+4] = input[i:i+4] ^ key

if (input == "into the rabbit hole"):
    print("Correct! you may enter..")

```
</div>

**פיצוח:**
מכיוון שפעולת XOR מול אותו מפתח מבטלת את עצמה (Involution), הפעלתי סקריפט Python שלוקח את מחרוזת היעד ומבצע עליה XOR מול המפתח "ABRA" כדי לשחזר את הקלט המקורי הנדרש.

**סקריפט Python:**

<div dir="ltr" align="left">

```python
target = "into the rabbit hole"
key = "ABRA"
flag = ""

for i in range(len(target)):
    char_xor = ord(target[i]) ^ ord(key[i % 4])
    flag += chr(char_xor)

print(f"The exact input required is: {flag}")

```
</div>

**תוצאה:** הסיסמה המחושבת היא <span dir="ltr">`(,&.a6:$a03##+&a)->$`</span>. הזנת הסיסמה בתוכנית החזירה אימות מוצלח.

![Level 2 Success](images/l2_success.png)

---


## Level 3 - 

**מטרה:** ניתוח פונקציית אימות המתבססת על שליפת נתונים ממערך סמוי, במטרה למצוא את רצף המספרים הנכון לעקיפתה.

המשכתי לשלב הבא. הפעם, התוכנית הדפיסה בקשה שונה לחלוטין:

```text
..Wait! Can you help me first with something?
(Enter the correct numbers)

```

שוב השתמשתי בחלון ה-Strings ב-IDA כדי לאתר את המחרוזת, ודרכה הגעתי לפונקציית הקלט `sub_401430`.

![Level 3 Strings](images/l3_strings.png)

במקום לחפש קריאה ל-`fgets` כמו בשלב הקודם, הפעם זיהיתי לולאה שמשתמשת ב-`scanf` עם תבנית העיצוב `"%hu"`. המשמעות היא שהתוכנית מצפה לקבל סדרה של מספרים מסוג Unsigned Short (2 בתים כל אחד). לקראת סוף הלולאה, מצאתי את הפקודות `cmp edx, 8` ולאחריה `jb` (Jump if Below). זוהי בדיקת גבולות שמוודאת שהמשתמש הזין בדיוק 8 אינדקסים חוקיים, בטווח שבין 0 ל-7.

![Scan Loop Part 1](images/l3_scan_loop0.png)
![Scan Loop Part 1](images/l3_scan_loop1.png)

לאחר קליטת 8 המספרים אל תוך מערך, התוכנית מבצעת קריאה (`call`) לפונקציה נפרדת, `sub_4014F0`, שמתפקדת כ"שופט" (Judge).

נכנסתי לפונקציית ה"שופט" וניתחתי את הלוגיקה שלה. הפונקציה עוברת על כל אחד מ-8 המספרים שהזנו, ומשתמשת בהם כ**אינדקסים** לגישה למערך נתונים סמוי (Data Array) שנמצא בזיכרון בכתובת `404000`.
פעולת השליפה: `movsx ecx, word ptr asc_404000[eax*2]`.
השימוש בפקודת `movsx` (Move with Sign-Extension) הוא רמז קריטי – הוא מעיד על כך שהנתונים השמורים במערך הם מספרים חתומים (Signed) המיוצגים בשיטת המשלים ל-2.

![Scan Loop Part 1](images/l3_scan_loop2.png)

מיד לאחר השליפה, מתבצעת הפקודה `cmp ecx, edx` כדי להשוות את הערך החדש שנשלף לערך שנשלף באיטרציה הקודמת, ולאחריה `jg` (Jump if Greater). אם הערך הנוכחי קטן מהקודם, הלולאה נשברת והאימות נכשל. מכאן הסקתי שהשופט יאשר את הקלט רק אם הערכים שיישלפו מהמערך הסמוי ייצרו סדרה **מונוטונית עולה**.

**אזור ה- Data Extraction:**
כדי להרכיב את רצף האינדקסים הנכון, קפצתי לחלון ה-Hex View בכתובת `404000` כדי לחלץ את המערך.

![Hex View Data](images/l3_hex_view.png)

חילצתי את 8 האיברים (Word = 2 Bytes), המרתי אותם מ-Little Endian, והתייחסתי לסיבית הסימן כפי שהורתה פקודת ה-movsx. התקבל המערך הבא:
`[7, 33, 1, -600, -5000, 1777, 13, 69]`

**פסאודו-קוד של פונקציית ה-Judge:**

<div dir="ltr" align="left">

```text
secret_array = [7, 33, 1, -600, -5000, 1777, 13, 69]
prev_val = secret_array[input[0]]

for i from 1 to 7:
    curr_val = secret_array[input[i]]
    if curr_val < prev_val:
        return FAIL
    prev_val = curr_val

return SUCCESS

```
</div>

**פיצוח:**
כדי שהנתונים יישלפו בסדר עולה מהקטן ביותר (-5000) לגדול ביותר (1777), כתבתי סקריפט פייתון שקורא את המערך המקורי, ממיין אותו לפי הערכים, ומחזיר את האינדקסים המקוריים לפי הסדר הממוין.

**סקריפט Python:**

<div dir="ltr" align="left">

```python
secret = [7, 33, 1, -600, -5000, 1777, 13, 69]

# Map each value to its original index, then sort by value
indexed_array = [(val, idx) for idx, val in enumerate(secret)]
indexed_array.sort(key=lambda x: x[0])

solution = [idx for val, idx in indexed_array]
print(f"The required index sequence is: {solution}")

```
</div>

**תוצאה:** סדר האינדקסים הנדרש הוא (מימין לשמאל): `4, 3, 2, 0, 6, 1, 7, 5`. הזנת המספרים הובילה להודעת הניצחון.

![Level 3 Success](images/l3_success.png)
