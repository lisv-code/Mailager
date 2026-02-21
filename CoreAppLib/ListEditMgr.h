#pragma once
#ifndef _LIS_LIST_EDIT_MANAGER_H_
#define _LIS_LIST_EDIT_MANAGER_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ListEditMgr_Def
{
	enum class EditState { None = 0, Created, Modified, Deleted };
}

template <typename TItemData, typename TItemId>
class ListEditMgr
{
public:
	typedef std::function<TItemId(const TItemData&)> ItemIdGetter;
	typedef std::function<bool(const TItemData&, ListEditMgr_Def::EditState)> ItemDataProc;

	ListEditMgr(ItemIdGetter item_id_getter);

	void Clear();
	size_t GetItemCount() const;
	bool EnumItems(ItemDataProc item_proc);
	TItemData* AddItem(const TItemData& item_data, ListEditMgr_Def::EditState edit_state);
	TItemData* FindItem(const TItemId& item_id, const TItemData* skip_item = nullptr);
	bool SetItemModified(const TItemId& item_id);
	bool DeleteItem(const TItemId& item_id);
	std::unordered_map<ListEditMgr_Def::EditState, size_t> GetEditState(bool include_unchanged = false) const;
	bool ApplyChanges(ItemDataProc item_proc, std::vector<TItemData>& save_items, std::vector<TItemId>& del_ids);

private:
	ItemIdGetter itemIdGetter;
	std::vector<std::pair<TItemData, ListEditMgr_Def::EditState>> items;

	int FindItemIndex(const TItemId& item_id, const TItemData* skip_item) const;
};

// ********************************** ListEditMgr implementation ***********************************

template<typename TItemData, typename TItemId>
ListEditMgr<TItemData, TItemId>::ListEditMgr(ItemIdGetter item_id_getter)
	: itemIdGetter(item_id_getter)
{ }

template<typename TItemData, typename TItemId>
void ListEditMgr<TItemData, TItemId>::Clear()
{
	items.clear();
}

template<typename TItemData, typename TItemId>
inline size_t ListEditMgr<TItemData, TItemId>::GetItemCount() const
{
	return items.size();
}

template<typename TItemData, typename TItemId>
bool ListEditMgr<TItemData, TItemId>::EnumItems(ItemDataProc item_proc)
{
	if (item_proc)
		for (const auto& item : items)
			if (!item_proc(item.first, item.second))
				return false;
	return true;
}

template<typename TItemData, typename TItemId>
TItemData* ListEditMgr<TItemData, TItemId>::AddItem(const TItemData& item_data, ListEditMgr_Def::EditState edit_state)
{
	items.emplace_back(item_data, edit_state);
	return &items.back().first;
}

template<typename TItemData, typename TItemId>
TItemData* ListEditMgr<TItemData, TItemId>::FindItem(const TItemId& item_id, const TItemData* skip_item)
{
	int idx = FindItemIndex(item_id, skip_item);
	return idx >= 0 ? &items[idx].first : nullptr;
}

template<typename TItemData, typename TItemId>
bool ListEditMgr<TItemData, TItemId>::SetItemModified(const TItemId& item_id)
{
	int idx = FindItemIndex(item_id, nullptr);
	if ((idx >= 0) && (ListEditMgr_Def::EditState::None == items[idx].second)) {
		items[idx].second = ListEditMgr_Def::EditState::Modified;
		return true;
	}
	return false;
}

template<typename TItemData, typename TItemId>
bool ListEditMgr<TItemData, TItemId>::DeleteItem(const TItemId& item_id)
{
	int idx = FindItemIndex(item_id, nullptr);
	if (idx < 0) return false;
	bool is_item_persisted = (ListEditMgr_Def::EditState::None == items[idx].second)
		|| (ListEditMgr_Def::EditState::Modified == items[idx].second);
	if (is_item_persisted)
		items[idx].second = ListEditMgr_Def::EditState::Deleted;
	else
		items.erase(items.begin() + idx);
	return true;
}

template<typename TItemData, typename TItemId>
std::unordered_map<ListEditMgr_Def::EditState, size_t>
	ListEditMgr<TItemData, TItemId>::GetEditState(bool include_unchanged) const
{
	std::unordered_map<ListEditMgr_Def::EditState, size_t> result;
	for (const auto& item : items)
		if (include_unchanged || (ListEditMgr_Def::EditState::None != item.second))
			++result[item.second];
	return result;
}

template<typename TItemData, typename TItemId>
bool ListEditMgr<TItemData, TItemId>::ApplyChanges(ItemDataProc item_proc,
	std::vector<TItemData>& save_items, std::vector<TItemId>& del_ids)
{
	for (const auto& item : items) {
		if (item_proc && !item_proc(item.first, item.second))
			return false;
		if (ListEditMgr_Def::EditState::Deleted != item.second) save_items.push_back(item.first);
		else del_ids.push_back(itemIdGetter(item.first));
	}
	return true;
}

template <typename TItemData, typename TItemId>
int ListEditMgr<TItemData, TItemId>::FindItemIndex(const TItemId& item_id, const TItemData* skip_item) const
{
	for (size_t i = 0; i < items.size(); ++i) {
		if (skip_item && (skip_item == &items[i].first))
			continue;
		if (item_id == itemIdGetter(items[i].first))
			return (int)i;
	}
	return -1;
}

#endif // #ifndef _LIS_LIST_EDIT_MANAGER_H_
