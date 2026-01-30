"""
Automatic Weed Remover using YOLOv9
Complete End-to-End Research Implementation

Author: <Your Name>
Paper: Automatic Weed Remover (YOLOv9 based)

This script performs:
1. Training
2. Evaluation
3. Inference
4. Visualization
"""

import os
import cv2
import pandas as pd
import matplotlib.pyplot as plt
from ultralytics import YOLO


# -------------------------------
# CONFIGURATION
# -------------------------------
DATA_YAML = "weed.yaml"
MODEL_NAME = "yolov9c.pt"
PROJECT_NAME = "Automatic_Weed_Remover"
EPOCHS = 50
IMG_SIZE = 640
BATCH_SIZE = 16
DEVICE = 0   # 0 = GPU, "cpu" for CPU


# -------------------------------
# 1. TRAINING
# -------------------------------
def train_model():
    print("\n[INFO] Starting YOLOv9 Training...")
    model = YOLO(MODEL_NAME)

    model.train(
        data=DATA_YAML,
        epochs=EPOCHS,
        imgsz=IMG_SIZE,
        batch=BATCH_SIZE,
        device=DEVICE,
        project="runs",
        name=PROJECT_NAME
    )

    print("[INFO] Training Completed!")


# -------------------------------
# 2. EVALUATION
# -------------------------------
def evaluate_model():
    print("\n[INFO] Evaluating Model...")
    model = YOLO(f"runs/{PROJECT_NAME}/weights/best.pt")
    metrics = model.val()

    print("\n===== PERFORMANCE METRICS =====")
    print("Precision (P): ", metrics.box.map)
    print("Recall (R): ", metrics.box.mar)
    print("mAP@50: ", metrics.box.map50)
    print("mAP@50-95: ", metrics.box.map)
    print("================================")


# -------------------------------
# 3. INFERENCE
# -------------------------------
def run_inference(source="test_field.jpg"):
    print("\n[INFO] Running Inference...")
    model = YOLO(f"runs/{PROJECT_NAME}/weights/best.pt")
    results = model.predict(source=source, show=True)
    print("[INFO] Inference Done!")


# -------------------------------
# 4. VISUALIZATION
# -------------------------------
def visualize_results():
    print("\n[INFO] Visualizing Training Curves...")

    results_file = f"runs/{PROJECT_NAME}/results.csv"
    if not os.path.exists(results_file):
        print("[ERROR] results.csv not found!")
        return

    data = pd.read_csv(results_file)

    # Box Loss
    plt.figure()
    plt.plot(data["epoch"], data["train/box_loss"], label="Train Box Loss")
    plt.plot(data["epoch"], data["val/box_loss"], label="Val Box Loss")
    plt.xlabel("Epoch")
    plt.ylabel("Loss")
    plt.title("Box Loss Curve")
    plt.legend()
    plt.show()

    # Precision Recall
    plt.figure()
    plt.plot(data["epoch"], data["metrics/precision(B)"], label="Precision")
    plt.plot(data["epoch"], data["metrics/recall(B)"], label="Recall")
    plt.xlabel("Epoch")
    plt.ylabel("Score")
    plt.title("Precision & Recall Curve")
    plt.legend()
    plt.show()

    print("[INFO] Visualization Complete!")


# -------------------------------
# 5. MAIN PIPELINE
# -------------------------------
if __name__ == "__main__":
    print("=====================================")
    print(" AUTOMATIC WEED REMOVER - YOLOv9 ")
    print("=====================================")

    print("\nChoose an option:")
    print("1 - Train Model")
    print("2 - Evaluate Model")
    print("3 - Run Inference on Image/Video")
    print("4 - Visualize Training Curves")

    choice = input("Enter your choice: ")

    if choice == "1":
        train_model()
    elif choice == "2":
        evaluate_model()
    elif choice == "3":
        src = input("Enter image/video path: ")
        run_inference(src)
    elif choice == "4":
        visualize_results()
    else:
        print("Invalid choice!")
