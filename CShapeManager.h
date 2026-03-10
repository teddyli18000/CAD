#pragma once

#include "CLine.h"
#include "CViewTransform.h"
#include "Command.h"
#include <memory>
#include <stack>
#include <string>
#include <vector>

// shape manager
class CShapeManager {
private:
    std::vector<std::shared_ptr<CLine>> m_shapes;
    std::stack<std::unique_ptr<ICadCommand>> m_undoStack;
    std::stack<std::unique_ptr<ICadCommand>> m_redoStack;
    int m_historyIndex = 0;
    int m_savedHistoryIndex = 0;

public:
    void AddShape(std::shared_ptr<CLine> shape);// add
    void RemoveShape(std::shared_ptr<CLine> shape);//remove
    //清空图元与命令栈
    void Clear();

    //获取可写图元列表
    std::vector<std::shared_ptr<CLine>>& GetShapes();

    //获取只读图元列表
    const std::vector<std::shared_ptr<CLine>>& GetShapes() const;

    //绘制全部图元
    void DrawAll(CDC* pDC, const CViewTransform& transform, bool bShowPoints) const;

    //执行命令
    void ExecuteCommand(std::unique_ptr<ICadCommand> cmd);

    //undo
    void Undo();

    //redo
    void Redo();

    //save to DXF
    bool SaveToDXF(const std::wstring& filepath) const;

    //load from DXF
    bool LoadFromDXF(const std::wstring& filepath);

    //标记当前状态为已保存
    void MarkSaved();

    //是否存在未保存修改
    bool HasUnsavedChanges() const;
};
