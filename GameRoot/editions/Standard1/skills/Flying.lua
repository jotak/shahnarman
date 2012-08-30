icon = "ane"

function getName()
	if language == "french" then
		return "Volant"
	else
		return "Flying"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité peut se déplacer sur tous les types de terrain sans pénalité. De plus, seules les unités volantes ou attaquant à distance peuvent attaquer une autre unité volante."
	else
		return "The unit can move on any kind of terrain without penalty. Moreover, only flying or range-attacking units can atack another flying unit."
	end
end

function isBattleValid(me, isRange, isAttacking, other)
	if isAttacking == 0 and isRange == 0 then
		-- I am defending, and the attacker uses melee attack ; check that he can fly
		objtype, player, unit = split(other, " ")
		skills = {getSkillsList(player, unit)}
		size = table.getn(skills)
		for i = 1, size do
			skill = skills[i]
			name = getSkillData(player, unit, skill)
			if name == "Flying" then
				return 1
			end
		end
		return 0
	end
	return 1
end

function getMod_movecost_plain(value)
	return 1
end

function getMod_movecost_toundra(value)
	return 1
end

function getMod_movecost_forest(value)
	return 1
end

function getMod_movecost_mountain(value)
	return 1
end

function getMod_movecost_desert(value)
	return 1
end

function getMod_movecost_sea(value)
	return 1
end
