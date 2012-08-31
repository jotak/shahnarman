cost = {0,3,0,0}
icon = "crusade"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Loi du champ de bataille"
	else
		return "Law of battle field"
	end
end

function getDescription()
	if language == "french" then
		return "Toutes les unités perdent la capacité Initiative."
	else
		return "All units loose the skill First strike."
	end
end

function onCast()
end

function onResolve()
	attachAsGlobal()
end

function onNewTurn()
	players = {getPlayersList()}
	table.foreach(players, function(i, player)
		units = {getUnitsList(player)}
		table.foreach(units, function(j, unit)
			skills = {getSkillsList(player, unit)}
			table.foreach(skills, function(k, skill)
				name = getSkillData(player, unit, skill)
				if name == "FirstStrike" then
					deactivateSkill(player, unit, skill)
				end
			end)
		end)
	end)
end
