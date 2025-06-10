import pdfplumber

total_chars = 0
with pdfplumber.open("data/Master_Thesis_S2025.pdf") as pdf:
    for page in pdf.pages:
        text = page.extract_text()
        if text:
            total_chars += len(text)

print("Total characters (including spaces):", total_chars)