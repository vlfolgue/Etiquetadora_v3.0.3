import pandas as pd
import matplotlib.pyplot as plt
from tkinter import Tk, filedialog
import os

# Ocultar ventana principal de Tkinter
root = Tk()
root.withdraw()

# Seleccionar archivo CSV
file_path = filedialog.askopenfilename(
    title="Selecciona el archivo CSV de lecturas FC",
    filetypes=[("CSV files", "*.csv"), ("Todos los archivos", "*.*")]
)

if not file_path:
    print("❌ No se seleccionó ningún archivo.")
    exit()

# Leer CSV sin encabezado
df = pd.read_csv(file_path, header=None, names=["Indice", "FC1", "FC2"])

# Calcular umbrales sugeridos
umbral_fc1 = (df["FC1"].min() + df["FC1"].max()) // 2
umbral_fc2 = (df["FC2"].min() + df["FC2"].max()) // 2

print("🔎 Umbral sugerido FC1:", umbral_fc1)
print("🔎 Umbral sugerido FC2:", umbral_fc2)

# Crear gráfico de dispersión (scatter)
plt.figure(figsize=(12, 6))
plt.scatter(df["Indice"], df["FC1"], label="FC1", s=8, color='blue')
plt.scatter(df["Indice"], df["FC2"], label="FC2", s=8, color='orange')
plt.axhline(y=umbral_fc1, color='blue', linestyle='--', label=f'Threshold FC1 ({umbral_fc1})')
plt.axhline(y=umbral_fc2, color='orange', linestyle='--', label=f'Threshold FC2 ({umbral_fc2})')

plt.title("Lecturas de Fotocélulas FC1 y FC2 (modo puntos)")
plt.xlabel("Muestra")
plt.ylabel("Valor analógico")
plt.grid(True)
plt.legend()
plt.tight_layout()

# Guardar imagen en la misma carpeta
output_path = os.path.join(os.path.dirname(file_path), "grafico_fc.png")
plt.savefig(output_path)
print(f"📷 Gráfico guardado en: {output_path}")

plt.show()

