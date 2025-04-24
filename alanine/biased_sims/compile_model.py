import torch
import sys
import os
import shutil

def compile_model(input_path, output_path=None, use_trace=False, sample_input=None):
    """
    将PyTorch模型编译为TorchScript格式
    """
    if output_path is None:
        base, _ = os.path.splitext(input_path)
        output_path = base + ".ptc"
    
    print(f"处理模型: {input_path}")
    
    # 检查是否已经是TorchScript格式
    try:
        # 尝试作为TorchScript加载
        model = torch.jit.load(input_path, map_location=torch.device('cpu'))
        print(f"检测到输入已经是TorchScript格式，直接复制到输出路径")
        # 如果成功，直接复制文件
        shutil.copy(input_path, output_path)
        print(f"模型已复制到: {output_path}")
        return True
    except RuntimeError:
        # 如果不是TorchScript，则尝试正常加载并编译
        try:
            print("模型不是TorchScript格式，尝试加载并编译...")
            model = torch.load(input_path, map_location=torch.device('cpu'), weights_only=False)
            model.eval()
            
            if use_trace:
                if sample_input:
                    values = list(map(float, sample_input.split(',')))
                    example = torch.tensor([values], dtype=torch.float32)
                    print(f"使用样本输入 {example.shape}: {example}")
                    script_model = torch.jit.trace(model, example)
                else:
                    raise ValueError("使用trace模式时必须提供sample_input")
            else:
                print("使用torch.jit.script编译模型")
                script_model = torch.jit.script(model)
            
            # 保存模型
            script_model.save(output_path)
            print(f"编译后的模型已保存到: {output_path}")
            return True
        except Exception as e:
            print(f"错误: {e}")
            return False

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="编译或复制PyTorch模型为TorchScript格式")
    parser.add_argument("input", help="输入模型路径(.pt或.pth)")
    parser.add_argument("--output", "-o", help="输出模型路径(.pt或.ptc)")
    parser.add_argument("--trace", "-t", action="store_true", help="使用trace替代script")
    parser.add_argument("--sample-input", "-s", help="使用trace时的样本输入 (逗号分隔的值)")
    
    args = parser.parse_args()
    
    if args.trace and not args.sample_input:
        print("错误: 使用--trace时必须提供--sample-input")
        sys.exit(1)
    
    compile_model(args.input, args.output, args.trace, args.sample_input)