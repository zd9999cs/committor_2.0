#!/usr/bin/env python
# filepath: model_benchmark.py

import torch
import time
import numpy as np
import argparse
import os
import matplotlib.pyplot as plt
from tqdm import tqdm

def benchmark_model(model_path, input_size=45, batch_size=1, num_runs=1000, 
                   warmup=100, device='auto', save_results=True):
    """
    对PyTorch模型进行基准测试
    
    参数:
        model_path: 模型路径
        input_size: 输入特征数量
        batch_size: 批处理大小
        num_runs: 测试运行次数
        warmup: 预热运行次数
        device: 使用的设备 ('cpu', 'cuda', 'auto')
        save_results: 是否保存结果
    """
    # 确定设备
    if device == 'auto':
        device = 'cuda' if torch.cuda.is_available() else 'cpu'
        
    print(f"测试模型: {model_path}")
    print(f"设备: {device}")
    print(f"输入大小: {input_size}")
    print(f"批处理大小: {batch_size}")
    print(f"运行次数: {num_runs} (预热 {warmup}次)")
    
    # 加载模型
    try:
        model = torch.jit.load(model_path, map_location=device)
        model_type = "TorchScript"
    except Exception:
        try:
            model = torch.load(model_path, map_location=device, weights_only=False)
            model.eval()
            model_type = "PyTorch"
        except Exception as e:
            print(f"无法加载模型: {e}")
            return
    
    print(f"模型类型: {model_type}")
    
    # 创建模拟输入
    dummy_input = torch.rand(batch_size, input_size, device=device)
    
    # 预热
    print("预热中...")
    with torch.no_grad():
        for _ in range(warmup):
            _ = model(dummy_input)
            
    # 计时
    times = []
    print("测试中...")
    with torch.no_grad():
        for _ in tqdm(range(num_runs)):
            start_time = time.time()
            _ = model(dummy_input)
            
            # 确保GPU操作完成
            if device == 'cuda':
                torch.cuda.synchronize()
                
            end_time = time.time()
            times.append((end_time - start_time) * 1000)  # 转换为毫秒
    
    # 计算统计数据
    times = np.array(times)
    mean_time = np.mean(times)
    median_time = np.median(times)
    std_time = np.std(times)
    min_time = np.min(times)
    max_time = np.max(times)
    p99_time = np.percentile(times, 99)
    
    # 输出结果
    print("\n性能结果:")
    print(f"平均推理时间: {mean_time:.4f} ms")
    print(f"中位数推理时间: {median_time:.4f} ms")
    print(f"标准差: {std_time:.4f} ms")
    print(f"最小时间: {min_time:.4f} ms")
    print(f"最大时间: {max_time:.4f} ms")
    print(f"P99时间: {p99_time:.4f} ms")
    print(f"吞吐量: {1000 * batch_size / mean_time:.2f} 样本/秒")
    
    if save_results:
        # 保存结果到CSV
        result_dir = "benchmark_results"
        os.makedirs(result_dir, exist_ok=True)
        
        model_name = os.path.basename(model_path)
        result_file = os.path.join(result_dir, f"{model_name}_{device}_{batch_size}.csv")
        
        with open(result_file, 'w') as f:
            f.write("run,time_ms\n")
            for i, t in enumerate(times):
                f.write(f"{i},{t:.6f}\n")
        
        # 绘制直方图
        plt.figure(figsize=(10, 6))
        plt.hist(times, bins=50, alpha=0.7)
        plt.axvline(mean_time, color='r', linestyle='dashed', linewidth=1, label=f'平均: {mean_time:.2f}ms')
        plt.axvline(median_time, color='g', linestyle='dashed', linewidth=1, label=f'中位数: {median_time:.2f}ms')
        plt.axvline(p99_time, color='b', linestyle='dashed', linewidth=1, label=f'P99: {p99_time:.2f}ms')
        plt.xlabel('推理时间 (ms)')
        plt.ylabel('频率')
        plt.title(f'模型推理时间分布 - {model_name} on {device}')
        plt.legend()
        plt.grid(True, alpha=0.3)
        
        plot_file = os.path.join(result_dir, f"{model_name}_{device}_{batch_size}.png")
        plt.savefig(plot_file)
        print(f"结果已保存到 {result_file} 和 {plot_file}")
    
    return {
        "mean": mean_time,
        "median": median_time,
        "std": std_time,
        "min": min_time,
        "max": max_time,
        "p99": p99_time,
        "throughput": 1000 * batch_size / mean_time
    }

def compare_models(models, input_size=45, batch_sizes=[1, 8, 32], 
                  devices=['cpu', 'cuda'] if torch.cuda.is_available() else ['cpu']):
    """比较多个模型在不同条件下的性能"""
    results = {}
    
    for model_path in models:
        model_name = os.path.basename(model_path)
        results[model_name] = {}
        
        for device in devices:
            results[model_name][device] = {}
            
            for batch_size in batch_sizes:
                print(f"\n{'='*50}")
                print(f"测试 {model_name} 在 {device} 上使用批量大小 {batch_size}")
                print(f"{'='*50}")
                
                result = benchmark_model(
                    model_path, 
                    input_size=input_size,
                    batch_size=batch_size,
                    device=device
                )
                
                results[model_name][device][batch_size] = result
    
    # 打印比较表格
    print("\n\n性能比较表格:")
    print("="*80)
    print(f"{'模型':<20} {'设备':<10} {'批量':<8} {'平均(ms)':<12} {'中位数(ms)':<12} {'样本/秒':<12}")
    print("-"*80)
    
    for model_name in results:
        for device in results[model_name]:
            for batch_size in results[model_name][device]:
                r = results[model_name][device][batch_size]
                print(f"{model_name:<20} {device:<10} {batch_size:<8} "
                      f"{r['mean']:<12.4f} {r['median']:<12.4f} {r['throughput']:<12.2f}")
    
    return results

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="PyTorch模型推理性能基准测试")
    parser.add_argument("model", help="模型路径", nargs='+')
    parser.add_argument("--input-size", "-i", type=int, default=45, help="输入特征数量")
    parser.add_argument("--batch-sizes", "-b", type=int, nargs='+', default=[1, 8, 32], 
                        help="要测试的批处理大小")
    parser.add_argument("--device", "-d", choices=['cpu', 'cuda', 'auto'], default='auto',
                       help="使用的设备")
    parser.add_argument("--runs", "-r", type=int, default=1000, help="测试运行次数")
    parser.add_argument("--warmup", "-w", type=int, default=100, help="预热运行次数")
    parser.add_argument("--compare", "-c", action="store_true", help="比较多个模型")
    
    args = parser.parse_args()
    
    # 修复这里的bug，处理auto设备选择
    if args.compare:
        # 修复的代码：当device为auto时使用默认设备列表
        if args.device == 'auto':
            devices = ['cpu', 'cuda'] if torch.cuda.is_available() else ['cpu']
        else:
            devices = [args.device]
            
        compare_models(args.model, args.input_size, args.batch_sizes, devices)
    else:
        for model_path in args.model:
            benchmark_model(model_path, args.input_size, args.batch_sizes[0], 
                          args.runs, args.warmup, args.device)